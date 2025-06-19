#include "Processors.h"
#include "BinaryUtils.h"
#include "instructions/FloatingPoint.h"
#include "instructions/KInstructions.h"

Hardware::Coprocessor::Coprocessor(Machine& machine) : registerFile{0}, machine(machine) {}

Hardware::FloatingPointUnit::FloatingPointUnit(Machine& machine) : Coprocessor(machine), FPcond(false) {}

std::unique_ptr<Hardware::Instruction> Hardware::FloatingPointUnit::decode(const Word& binary_instruction) {
    using namespace Binary;

    Word& programCounter = machine.accessCPU().accessProgramCounter();

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
    else return std::make_unique<instr<Double>>( getDouble(ft), getDouble(fs), getDouble(fd) );
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

Hardware::SystemControlUnit::SystemControlUnit(Machine& machine) : Coprocessor(machine) {}

std::unique_ptr<Hardware::Instruction> Hardware::SystemControlUnit::decode(const Word& binary) {
    using namespace Binary;

    const Byte rs = (binary >> 21) & 0x1F;
    const Byte rt = (binary >> 16) & 0x1F;
    const Byte rd = (binary >> 11) & 0x1F;
    const Funct funct = Funct(binary & 0x3F);

    auto& statusReg = machine.accessCoprocessor(0)->accessRegister(STATUS).ui;

    if (rs == 0x10 && funct == ERET) return std::make_unique<ExceptionReturn>(machine);
    if (rs == 0) return std::make_unique<MoveFromCoprocessor0>(statusReg, machine.accessCPU().accessRegister(rt).i, registerFile[rd].i );
    if (rs == 4) return std::make_unique<MoveToCoprocessor0>(statusReg, machine.readCPU().readRegister(rt).i, registerFile[rd].i );

    throw Trap(Trap::ExceptionCode::RI);
    return std::make_unique<NoOp>();
}
