#include "Processors.h"
#include "BinaryUtils.h"
#include "instructions/FloatingPoint.h"
#include "instructions/KInstructions.h"

Hardware::Coprocessor::Coprocessor(Machine& machine) : machine(machine), registerFile{0} {}

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

    if (fmt == BC) {
        if (ft) return std::make_unique<FPBranchOnTrue>(programCounter, FPcond, immediate);
        return std::make_unique<FPBranchOnFalse>(programCounter, FPcond, immediate);
    }

    #define FR_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[ft].f, registerFile[fs].f, registerFile[fd].f)
    #define FCMP_INIT(oc, instr) case oc: return std::make_unique<instr>(FPcond, registerFile[ft].f, registerFile[fs].f)
    switch (FPFunct(funct)) {
            FR_INIT(FPADD, FPAddSingle);
            FR_INIT(FPDIV, FPDivideSingle);
            FR_INIT(FPMUL, FPMultiplySingle);
            FR_INIT(FPSUB, FPSubtractSingle);
            FCMP_INIT(FPCEQ, FPCompareEqualSingle);
            FCMP_INIT(FPCLT, FPCompareLessThanSingle);
            FCMP_INIT(FPCLE, FPCompareLessThanOrEqualSingle);
            case FPABS: return std::make_unique<FPAbsoluteValueSingle>(registerFile[fd].f, registerFile[fs].f);
            default:
                // implement trap handler
                throw "Not implemnted yet FP";
        }

}

Hardware::SystemControlUnit::SystemControlUnit(Machine& machine) : Coprocessor(machine) {}

std::unique_ptr<Hardware::Instruction> Hardware::SystemControlUnit::decode(const Word& binary) {
    using namespace Binary;

    const Byte rs = (binary >> 21) & 0x1F;
    const Byte rt = (binary >> 16) & 0x1F;
    const Byte rd = (binary >> 11) & 0x1F;
    const Funct funct = Funct(binary & 0x3F);

    if (funct == ERET) return std::make_unique<ExceptionReturn>(machine);
    if (rs == 0) return std::make_unique<MoveFromCoprocessor0>(machine, machine.accessCPU().accessRegister(rt).i, registerFile[rd].i );
    if (rt == 4) return std::make_unique<MoveToCoprocessor0>(machine, machine.readCPU().readRegister(rt).i, registerFile[rd].i );


    throw 2;
    return nullptr;
}
