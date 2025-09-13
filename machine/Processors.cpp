#include "Processors.h"
#include "BinaryUtils.h"
#include "instructions/Instruction.h"


Hardware::WallClock::WallClock() {}
Hardware::WallClock::~WallClock() {
    stop();
}

void Hardware::WallClock::start(std::chrono::milliseconds quanta) {
    stop();

    duration = quanta;
    polling.store(true, std::memory_order_release);

    poller = std::thread([this] { run(); });
}

void Hardware::WallClock::stop() {
    bool expected = true;
    if (polling.compare_exchange_strong(expected, false, std::memory_order_acq_rel)) {
        // wake the sleeper immediately
        {
            std::lock_guard<std::mutex> lk(mx);
            // nothing else to change just notify
        }
        cv.notify_one();
    }
    if (poller.joinable()) poller.join();
}

void Hardware::WallClock::run() {
    using clock = std::chrono::steady_clock;
    auto next = clock::now() + duration;
    std::unique_lock<std::mutex> lock(mx);

    while (polling.load(std::memory_order_relaxed)) {
        cv.wait_until(lock, next, [this] () { return !polling.load(std::memory_order_relaxed); } );
        if (!polling.load(std::memory_order_relaxed)) break;

        IDactive.store(true, std::memory_order_release);

        next += duration;
    }

}

bool Hardware::WallClock::poll() {
    return IDactive.exchange(false, std::memory_order_acquire);
}

Hardware::TickClock::TickClock() : tickGoal(0), tickCount(0), ticking(false) {}

void Hardware::TickClock::start(std::chrono::milliseconds quanta) {
    tickGoal = quanta.count() * 1000;
    tickCount = 0;
    ticking = true;
}

void Hardware::TickClock::stop() {
    ticking = false;
}

bool Hardware::TickClock::poll() {
    if (tickCount++ > tickGoal) {
        tickCount = 0;
        return true;
    }
    return false;
}


Hardware::Processor::Processor(Core& core) : _core(&core), registerFile{0} {}

Hardware::FloatingPointUnit::FloatingPointUnit(Core& core) : Processor(core), FPcond(false) {}

std::unique_ptr<Hardware::Instruction> Hardware::FloatingPointUnit::decode(Word binary_instruction) {
    using namespace Binary;

    Word& programCounter = core().programCounter;

    // opcode is always the same
    FMT fmt = FMT((binary_instruction >> 21) & 0b11111);
    Byte ft = Byte((binary_instruction >> 16) & 0b11111);
    Byte fs = Byte((binary_instruction >> 11) & 0b11111);
    Byte fd = Byte((binary_instruction >> 6) & 0b11111);
    FPFunct funct = FPFunct(binary_instruction & 0b111111);
    short immediate = binary_instruction & 0xFFFF;

    // if (fmt == BC) {
    //     if (ft) return std::make_unique<FPBranchOnTrue>(programCounter, FPcond, immediate);
    //     return std::make_unique<FPBranchOnFalse>(programCounter, FPcond, immediate);
    // }

    #define FR_INIT(oc, instr) case oc: \
    assert(fmt == D || fmt == S); \
    if (fmt == S) return std::make_unique<instr<Single>>(registerFile[ft].f, registerFile[fs].f, registerFile[fd].f); \
    return std::make_unique<instr<Double>>( getDouble(ft), getDouble(fs), getDouble(fd) );
    // ensure if double that we are aligned to even regs (can't store double in any odd register)
    // maybe just replace with an exception later

    switch (funct) {
        FR_INIT(FPADD, FPAdd);
        FR_INIT(FPDIV, FPDivide);
        FR_INIT(FPMUL, FPMultiply);
        FR_INIT(FPSUB, FPSubtract);
        default:
            break;
    }

    throw Trap(Trap::ExceptionCode::RI);
    return std::make_unique<NoOp>();
}

Hardware::SystemControlUnit::SystemControlUnit(Core& core) : Processor(core) {}

std::unique_ptr<Hardware::Instruction> Hardware::SystemControlUnit::decode(Word binary) {
    using namespace Binary;

    const Byte rs = (binary >> 21) & 0x1F;
    const Byte rt = (binary >> 16) & 0x1F;
    const Byte rd = (binary >> 11) & 0x1F;
    const CP0Funct cp0funct = CP0Funct(binary & 0x3F);

    auto& statusReg = registerFile[STATUS].ui;

    if (rs == 0x10) {
        switch (cp0funct) {
            case ERET: return std::make_unique<ExceptionReturn>(core().machine());
            case TLBR: return std::make_unique<TLBReadIndexed>(statusReg, core().machine().memory.accessTLB(), registerFile[INDEX].ui, registerFile[ENTRYHI].ui, registerFile[ENTRYLO0].ui );
            case TLBWI: return std::make_unique<TLBWriteIndexed>(statusReg, core().machine().memory.accessTLB(), registerFile[INDEX].ui, registerFile[ENTRYHI].ui, registerFile[ENTRYLO0].ui );
            case TLBWR: return std::make_unique<TLBWriteRandom>(statusReg, core().machine().memory.accessTLB(), registerFile[RANDOM].ui, registerFile[ENTRYHI].ui, registerFile[ENTRYLO0].ui );
            case TLBP: return std::make_unique<TLBProbe>(statusReg, core().machine().memory.accessTLB(), registerFile[INDEX].ui, registerFile[ENTRYHI].ui);
            default: break;
        }
    }

    if (rs == 0) return std::make_unique<MoveFromCoprocessor0>(statusReg, core().iu.registerFile[rt].i, registerFile[rd].i );
    if (rs == 4) return std::make_unique<MoveToCoprocessor0>(statusReg, core().iu.registerFile[rt].i, registerFile[rd].i );

    throw Trap(Trap::ExceptionCode::RI);
}


Hardware::IntegerUnit::IntegerUnit(Core& core) : Processor(core) {}

Hardware::Core::Core(Machine& machine) : _machine(&machine), iu(*this), fpu(*this), scu(*this) {}

void Hardware::Core::cycle() {
    try {
        const Word& statusReg = scu.registerFile[Binary::STATUS].ui;
        // We use a oldIE and newIE comparison to have a sort of atomic interrupt
        // If an instruction flips the IE, it should not be possible for an interrupt to occur on the same instruction for (at least now) simplicity reasons
        // This is mainly useful for the worst-case when an interrupt signal comes right during a syscall or some other exception
        // It could be possible to work with less scaffolding for performance purposes, but for now this is ok
        
        bool oldIE = statusReg & 1;

        auto& instr = instructionCache[programCounter];
        if (!instr) instr = iu.decode( machine().memory.getWord(programCounter) );

        instr->run();
        programCounter += 4;

        bool newIE = statusReg & 1;

        if (interDev && interDev->poll() && oldIE && newIE) machine().raiseTrap( Trap::INTERRUPT, 0 ) ; // only run if the IE's are active and no action on the IE has been made

        // For every other exception, they are thrown during the instr->run() dynamic dispatch call, which prevents PC from incrementing
        // However, since we check the device (and therefore throw) after PC increment, the actual instruction at the new PC won't get run
        // Unless the kernel re-decrements the EPC by 4
        
    } catch (const Hardware::Trap& trap) {
        machine().raiseTrap(trap.exceptionCode, trap.badAddr);
    }
}

std::unique_ptr<Hardware::Instruction> Hardware::IntegerUnit::decode(Word binary_instruction) {
    // DECODE


    // Simplify local namespace

    using namespace Binary;

    auto& cor = core();
    auto& scu = cor.scu;
    auto& fpu = cor.fpu;
    auto& machine = cor.machine();
    
    Opcode opcode = Opcode((binary_instruction >> 26) & 0b111111);  // For All

    if (opcode == FP_TYPE) return fpu.decode(binary_instruction);
    if (opcode == K_TYPE) return scu.decode(binary_instruction);

    auto& RAM = machine.memory;
    auto& statusReg = scu.registerFile[STATUS].ui;

    Word address = binary_instruction & 0x1FFFFFF;                  // For Jump

    Register rs = Register((binary_instruction >> 21) & 0b11111);   // For I/R
    Register rt = Register((binary_instruction >> 16) & 0b11111);   // For I/R

    Register rd = Register((binary_instruction >> 11) & 0b11111);   // For R
    Byte shamt = (binary_instruction >> 6) & 0b11111;               // For R
    Funct funct = Funct(binary_instruction & 0b111111);             // For R

    short immediate = binary_instruction & 0xFFFF;                  // For I

    // Return instruction

    #define I_BRANCH_ZERO_INIT(oc, instr) case oc: return std::make_unique<instr>(immediate, cor.programCounter, registerFile[rs].i)
    if (opcode == REGIMM) {
        switch (rt) {
            I_BRANCH_ZERO_INIT(1, BranchOnGreaterThanOrEqualZero);
            I_BRANCH_ZERO_INIT(0, BranchOnLessThanZero);
            default:
                throw Trap(Trap::RI);
        }
    }

    if (opcode == SPECIAL2) {
        switch (funct) {
            case MUL: return std::make_unique<MultiplyToGPR>(registerFile[rd].i, registerFile[rt].i, registerFile[rs].i);
            
            default:
                throw Trap(Trap::RI);
        }
    }

    #define R_VAR_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rd].i, registerFile[rt].i, registerFile[rs].i)
    #define R_SHFT_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rd].i, registerFile[rt].i, shamt)
    #define HL_MOVE_INIT(oc, instr, reg) case oc: return std::make_unique<instr>(registerFile[reg].i, hiLo)
    #define HL_OP_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rs].i, registerFile[rt].i, hiLo)
    if (!opcode) {  // Is an R-Type Instruction
        switch (funct) {
            R_VAR_INIT(ADDU, AddUnsigned);
            R_VAR_INIT(AND, And);
            R_VAR_INIT(NOR, Nor);
            R_VAR_INIT(OR, Or);
            R_VAR_INIT(SLT, SetLessThan);
            R_VAR_INIT(SLTU, SetLessThanUnsigned);
            R_VAR_INIT(MOVN, MoveOnNotZero);
            R_SHFT_INIT(SLL, ShiftLeftLogical);
            R_SHFT_INIT(SRL, ShiftRightLogical);
            R_SHFT_INIT(SRA, ShiftRightArithmetic);
            R_VAR_INIT(SLLV, ShiftLeftLogicalVariable);
            R_VAR_INIT(SRLV, ShiftRightLogicalVariable);
            R_VAR_INIT(SRAV, ShiftRightArithmeticVariable);
            R_VAR_INIT(SUBU, SubtractUnsigned);
            R_VAR_INIT(XOR, Xor);
            HL_OP_INIT(MULT, Multiply);
            HL_OP_INIT(DIV, Divide);
            HL_OP_INIT(MULTU, MultiplyUnsigned);
            HL_OP_INIT(DIVU, DivideUnsigned);
            HL_MOVE_INIT(MFHI, MoveFromHi, rd);
            HL_MOVE_INIT(MFLO, MoveFromLo, rd);
            HL_MOVE_INIT(MTHI, MoveToHi, rs);
            HL_MOVE_INIT(MTLO, MoveToLo, rs);
            case ADD: return std::make_unique<Add>(registerFile[rd].i, registerFile[rt].i, registerFile[rs].i);
            case SUB: return std::make_unique<Subtract>(registerFile[rd].i, registerFile[rt].i, registerFile[rs].i);
            case SYSCALL: return std::make_unique<Syscall>();
            case SYNC: return std::make_unique<Sync>();
            case TEQ: return std::make_unique<TrapIfEqual>(registerFile[rs].i, registerFile[rt].i);
            case VMTUNNEL: return std::make_unique<VMTunnel>(statusReg, machine);
            case JR: return std::make_unique<JumpRegister>(cor.programCounter, registerFile[rs].i);
            case JALR: return std::make_unique<JumpAndLinkRegister>(cor.programCounter, registerFile[rd].i, registerFile[rs].i);
            default:
                throw Trap(Trap::RI);
        }
    }

    #define I_GEN_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rt].i, registerFile[rs].i, immediate)        // optimize instructions to use ui/i?
    #define I_MEM_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rt].i, registerFile[rs].i, immediate, RAM)
    #define I_BRANCH_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rt].i, registerFile[rs].i, immediate, cor.programCounter)
    #define FPMEM_INIT(oc, instr) case oc: return std::make_unique<instr>(fpu.registerFile[rt].f, registerFile[rs].i, immediate, RAM)
    switch (opcode) {
        I_GEN_INIT(ADDIU, AddImmediateUnsigned);
        I_GEN_INIT(ANDI, AndImmediate);
        I_GEN_INIT(SLTI, SetLessThanImmediate);
        I_GEN_INIT(XORI, XorImmediate);
        I_GEN_INIT(SLTIU, SetLessThanImmediateUnsigned);
        I_GEN_INIT(ORI, OrImmediate);
        I_MEM_INIT(LW, LoadWord);
        I_MEM_INIT(SW, StoreWord);
        I_MEM_INIT(SB, StoreByte);
        I_MEM_INIT(LBU, LoadByteUnsigned);
        I_MEM_INIT(LB, LoadByte);
        I_MEM_INIT(LHU, LoadHalfwordUnsigned);
        I_MEM_INIT(LH, LoadHalfword);
        I_MEM_INIT(SH, StoreHalfword);
        I_BRANCH_INIT(BEQ, BranchOnEqual);
        I_BRANCH_INIT(BNE, BranchOnNotEqual);
        FPMEM_INIT(LWC1, LoadFPSingle);
        FPMEM_INIT(SWC1, StoreFPSingle);
        I_BRANCH_ZERO_INIT(BGTZ, BranchOnGreaterThanZero);
        I_BRANCH_ZERO_INIT(BLEZ, BranchOnLessThanOrEqualZero);
        case ADDI: return std::make_unique<AddImmediate>(registerFile[rt].i, registerFile[rs].i, immediate);
        case J: return std::make_unique<Jump>(cor.programCounter, address);
        case JAL: return std::make_unique<JumpAndLink>(cor.programCounter, address, registerFile[RA].i);
        case LUI: return std::make_unique<LoadUpperImmediate>(registerFile[rt].i, immediate);
        case CACHE: return std::make_unique<PerformCacheOp>(statusReg, cor.instructionCache);
        default:
            throw Trap(Trap::RI);
    }

    throw Trap(Trap::RI);
}