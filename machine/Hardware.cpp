#include "Hardware.h"
#include "BinaryUtils.h"
#include "Instruction.h"
#include <iostream>
#include <iomanip>

Hardware::Machine::Machine() {
    for (int i = 0; i < 32; ++i) {
        registerFile[i] = 0.0f;
        fpRegisterFile[i] = 0.0f;
    }

    // temp for testing
    fpRegisterFile[1] = 3.14159f;
    fpRegisterFile[2] = 2.71828f;

    programCounter = 0x00400024;
    registerFile[Binary::SP] = 0x7fffffff;
    registerFile[Binary::GP] = 0x10008000; 
    killed = false;
}

const Word& Hardware::Machine::readProgramCounter() const {
    return programCounter;
}

const int& Hardware::Machine::readRegister(const Byte& reg) const {
    return registerFile[reg];
}

const Hardware::Memory& Hardware::Machine::readMemory() const {
    return RAM;
}

const float& Hardware::Machine::readFPRegister(const Byte& reg) const {
    return fpRegisterFile[reg];
}

void Hardware::Machine::loadInstructions(const std::vector<Word>& instructions) {
    // for right now, just load according to mips for no patricular reason
    // will figure out exact specifications later

    Word at = 0x00400024;
    for (const auto& instr : instructions) {
        RAM.setWord(at, instr);
        at += 4;
    }
    
    RAM.memoryBounds.textBound = at;
    RAM.memoryBounds.stackBound = 0x7fffe000;
    RAM.memoryBounds.dynamicBound = 0x70000000;
    RAM.memoryBounds.staticBound = 0; // tbd 
}

void Hardware::Machine::runInstruction() {
    // std::cout << "READING INSTUCTION: " << std::hex << RAM.getWord(programCounter) << std::endl;

    if (programCounter >= RAM.memoryBounds.textBound) {
        std::cout << "Reading past text memory. Killing process..." << std::endl;
        killed = true;
        return;
    }

    auto it = instructionCache.find(programCounter);
    if (it != instructionCache.end()) {
        it->second->run();
        programCounter += 4;
        return;
    }

    std::cout << "RUNNING INSTRUCTION: " << std::hex <<  RAM.getWord(programCounter) << std::endl;

    (
        instructionCache[programCounter] = instructionFactory( RAM.getWord(programCounter), programCounter, registerFile, fpRegisterFile, RAM, hiLo, killed )
    )->run(); // cool syntax

    programCounter += 4;
}

void Hardware::Machine::run() {
    while (!killed) runInstruction();
}

// TODO: Add better error handling

std::unique_ptr<Hardware::Instruction> Hardware::instructionFactory(const Word& binary_instruction, Word& programCounter, int* registerFile, float* fpRegisterFile, Hardware::Memory& RAM, Hardware::Machine::HiLoRegisters& hiLo, bool& kill_flag) {
    // DECODE
    using namespace Binary;
    
    Opcode opcode = Opcode((binary_instruction >> 26) & 0b111111);  // For All

    Word address = binary_instruction & 0x1FFFFFF;                  // For Jump

    Register rs = Register((binary_instruction >> 21) & 0b11111);   // For I/R
    Register rt = Register((binary_instruction >> 16) & 0b11111);   // For I/R

    Register rd = Register((binary_instruction >> 11) & 0b11111);   // For R
    Byte shamt = (binary_instruction >> 6) & 0b11111;               // For R
    Funct funct = Funct(binary_instruction & 0b111111);             // For R

    FMT fmt = FMT(rs);  // uses the same space, so just copy bits   // For FP
    std::cout << "RS: " << Word(rs) << ", FMT:" << Word(fmt) << '\n';
    Byte ft = Byte(rt);                                             // For FP
    Byte fs = Byte(rd);                                             // For FP
    Byte fd = Byte(shamt);                                          // For FP

    short immediate = binary_instruction & 0xFFFF;                  // For I

    // Return instruction

    #define R_VAR_INIT(x) std::make_unique<x>(registerFile[rd], registerFile[rt], registerFile[rs])
    #define R_SHFT_INIT(x) std::make_unique<x>(registerFile[rd], registerFile[rt], shamt)
    #define HL_MOVE_INIT(x, reg) std::make_unique<x>(registerFile[reg], hiLo)
    #define HL_OP_INIT(x) std::make_unique<x>(registerFile[rs], registerFile[rt], hiLo)
    if (!opcode) {  // Is an R-Type Instruction
        switch (funct) {
            case ADD:
                return R_VAR_INIT(Add);
            case ADDU:
                return R_VAR_INIT(AddUnsigned);
            case AND:
                return R_VAR_INIT(And);
            case NOR:
                return R_VAR_INIT(Nor);
            case OR:
                return R_VAR_INIT(Or);
            case SLT:
                return R_VAR_INIT(SetLessThan);
            case SLTU:
                return R_VAR_INIT(SetLessThanUnsigned);
            case SLL:
                return R_SHFT_INIT(ShiftLeftLogical);
            case SRL:
                return R_SHFT_INIT(ShiftRightLogical);
            case SRA:
                return R_SHFT_INIT(ShiftRightArithmetic);
            case SLLV:
                return R_VAR_INIT(ShiftLeftLogicalVariable);
            case SRLV:
                return R_VAR_INIT(ShiftRightLogicalVariable);
            case SRAV:
                return R_VAR_INIT(ShiftRightArithmeticVariable);
            case SUB:
                return R_VAR_INIT(Subtract);
            case SUBU:
                return R_VAR_INIT(SubtractUnsigned);
            case XOR:
                return R_VAR_INIT(Xor);
            case MULT:
                return HL_OP_INIT(Multiply);
            case DIV:
                return HL_OP_INIT(Divide);
            case MULTU:
                return HL_OP_INIT(MultiplyUnsigned);
            case DIVU:
                return HL_OP_INIT(DivideUnsigned);
            case MFHI:
                return HL_MOVE_INIT(MoveFromHi, rd);
            case MFLO:
                return HL_MOVE_INIT(MoveFromLo, rd);
            case MTHI:
                return HL_MOVE_INIT(MoveToHi, rs);
            case MTLO:
                return HL_MOVE_INIT(MoveToLo, rs);
            case JR:
                return std::make_unique<JumpRegister>(programCounter, registerFile[RA]);
            case JALR:
                return std::make_unique<JumpAndLinkRegister>(programCounter, registerFile[rd], registerFile[rs]);
            case SYSCALL:
                return std::make_unique<Syscall>(registerFile[V0], registerFile[A0], registerFile[A1], kill_flag);
            default:
                std::cout << "hello from bad funct " << std::hex << Word(binary_instruction) << std::endl;
                throw 1;
        }
    }

    #define FR_INIT(x) std::make_unique<x>(fpRegisterFile[ft], fpRegisterFile[fs], fpRegisterFile[fd])
    if (opcode == FP_TYPE) {
        switch (funct) {
            case FPADD:
                return FR_INIT(FPAddSingle);
            default:
                std::cout << "Not implemented yet" << std::endl;
                throw 3;
        }
    }

    #define I_GEN_INIT(x) std::make_unique<x>(registerFile[rt], registerFile[rs], immediate)
    #define I_MEM_INIT(x) std::make_unique<x>(registerFile[rt], registerFile[rs], immediate, RAM)
    #define I_BRANCH_INIT(x) std::make_unique<x>(registerFile[rt], registerFile[rs], immediate, programCounter)
    switch (opcode) {
        case ADDI:
            return I_GEN_INIT(AddImmediate);
        case ADDIU:
            return I_GEN_INIT(AddImmediateUnsigned);
        case ANDI:
            return I_GEN_INIT(AndImmediate);
        case SLTI:
            return I_GEN_INIT(SetLessThanImmediate);
        case SLTIU:
            return I_GEN_INIT(SetLessThanImmediateUnsigned);
        case ORI:
            return I_GEN_INIT(OrImmediate);
        case LW:
            return I_MEM_INIT(LoadWord);
        case SW:
            return I_MEM_INIT(StoreWord);
        case BEQ:
            return I_BRANCH_INIT(BranchOnEqual);
        case BNE:
            return I_BRANCH_INIT(BranchOnNotEqual);
        case J:
            return std::make_unique<Jump>(programCounter, address);
        case JAL:
            return std::make_unique<JumpAndLink>(programCounter, address, registerFile[RA]);
        case LUI:
            return std::make_unique<LoadUpperImmediate>(registerFile[rt], immediate);
        default:
            std::cout << "hello from bad opcode " << std::hex << Word(binary_instruction) << std::endl;
            throw 2;
    }

    throw 3;
    return nullptr;
    
}