#include "Processors.h"
#include "BinaryUtils.h"
#include "instructions/FloatingPoint.h"

Hardware::Coprocessor::Coprocessor(Machine& machine) : machine(machine), registerFile({0}) {}

inline const Hardware::reg32_t& Hardware::Coprocessor::readRegister(const Byte& reg) const {
    return registerFile[reg];
}

inline void Hardware::Coprocessor::moveToThis(const Byte& regDst, const reg32_t& valueSrc) {
    registerFile[regDst] = valueSrc;
}

Hardware::CP1::CP1(Machine& machine) : Coprocessor(machine), FPcond(false) {}

std::unique_ptr<Hardware::Instruction> Hardware::CP1::decode(const Word& binary_instruction, const Word& programCounter) {
    using namespace Binary;

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