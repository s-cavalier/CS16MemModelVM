#include "Instruction.h"
#include "BinaryInstruction.h"

// TODO: Add better error handling

std::unique_ptr<Hardware::Instruction> instructionFactory(const Word& binary_instruction, int& programCounter, int* registerFile, std::unordered_map<Word, int>& RAM) {
    // DECODE
    
    Opcode opcode = Opcode((binary_instruction >> 26) & 0b111111);  // For All

    Word address = binary_instruction & 0x1FFFFFF;                  // For Jump

    Register rs = Register((binary_instruction >> 21) & 0b11111);   // For I/R
    Register rt = Register((binary_instruction >> 16) & 0b11111);   // For I/R


    Register rd = Register((binary_instruction >> 11) & 0b11111);   // For R
    Byte shamt = (binary_instruction >> 6) & 0b11111;               // For R
    Funct funct = Funct(binary_instruction & 0b111111);             // For R

    short immediate = binary_instruction & 0xFFFF;                  // For I

    // Return instruction
    
    #define R_VAR_INIT(x) std::make_unique<x>(registerFile[rd], registerFile[rt], registerFile[rs])
    #define R_SHFT_INIT(x) std::make_unique<x>(registerFile[rd], registerFile[rt], shamt)
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
            case SUB:
                return R_VAR_INIT(Subtract);
            case SUBU:
                return R_VAR_INIT(SubtractUnsigned);
            case JR:
                return std::make_unique<JumpRegister>(programCounter, registerFile[RA]);
            case SYSCALL:
                return std::make_unique<Syscall>();
            default:
                throw 1;
        }
    }

    #define I_GEN_INIT(x) std::make_unique<x>(registerFile[rt], registerFile[rs], immediate);
    switch (opcode) {
        case J:

    }


}