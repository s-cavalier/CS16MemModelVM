#include "Hardware.h"
#include "Instruction.h"
#include "BinaryUtils.h"
#include <iostream>

// TODO: Add better error handling

std::unique_ptr<Hardware::Instruction> Hardware::instructionFactory(const Word& binary_instruction, Hardware::Machine::RegisterFile& registers, bool& kill_flag) {
    // DECODE

    // Simplify local namespace
    using namespace Binary;
    auto& RAM = registers.RAM;
    auto& registerFile = registers.registerFile;
    auto& fpRegisterFile = registers.fpRegisterFile;
    auto& programCounter = registers.programCounter;
    auto& FPcond = registers.FPcond;
    auto& hiLo = registers.hiLo;
    
    Opcode opcode = Opcode((binary_instruction >> 26) & 0b111111);  // For All

    Word address = binary_instruction & 0x1FFFFFF;                  // For Jump

    Register rs = Register((binary_instruction >> 21) & 0b11111);   // For I/R
    Register rt = Register((binary_instruction >> 16) & 0b11111);   // For I/R

    Register rd = Register((binary_instruction >> 11) & 0b11111);   // For R
    Byte shamt = (binary_instruction >> 6) & 0b11111;               // For R
    Funct funct = Funct(binary_instruction & 0b111111);             // For R

    FMT fmt = FMT(rs);  // uses the same space, so just copy bits   // For FP
    Byte ft = Byte(rt);                                             // For FP
    Byte fs = Byte(rd);                                             // For FP
    Byte fd = Byte(shamt);                                          // For FP

    short immediate = binary_instruction & 0xFFFF;                  // For I

    // Return instruction

    #define R_VAR_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rd], registerFile[rt], registerFile[rs])
    #define R_SHFT_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rd], registerFile[rt], shamt)
    #define HL_MOVE_INIT(oc, instr, reg) case oc: return std::make_unique<instr>(registerFile[reg], hiLo)
    #define HL_OP_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rs], registerFile[rt], hiLo)
    if (!opcode) {  // Is an R-Type Instruction
        switch (funct) {
            R_VAR_INIT(ADD, Add);
            R_VAR_INIT(ADDU, AddUnsigned);
            R_VAR_INIT(AND, And);
            R_VAR_INIT(NOR, Nor);
            R_VAR_INIT(OR, Or);
            R_VAR_INIT(SLT, SetLessThan);
            R_VAR_INIT(SLTU, SetLessThanUnsigned);
            R_SHFT_INIT(SLL, ShiftLeftLogical);
            R_SHFT_INIT(SRL, ShiftRightLogical);
            R_SHFT_INIT(SRA, ShiftRightArithmetic);
            R_VAR_INIT(SLLV, ShiftLeftLogicalVariable);
            R_VAR_INIT(SRLV, ShiftRightLogicalVariable);
            R_VAR_INIT(SRAV, ShiftRightArithmeticVariable);
            R_VAR_INIT(SUB, Subtract);
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
            case JR: return std::make_unique<JumpRegister>(programCounter, registerFile[RA]);
            case JALR: return std::make_unique<JumpAndLinkRegister>(programCounter, registerFile[rd], registerFile[rs]);
            case SYSCALL: return std::make_unique<Syscall>(registers, kill_flag);
            default:
                std::cout << "hello from bad funct " << std::hex << Word(binary_instruction) << std::endl;
                throw 1;
        }
    }

    #define FR_INIT(oc, instr) case oc: return std::make_unique<instr>(fpRegisterFile[ft], fpRegisterFile[fs], fpRegisterFile[fd])
    #define FCMP_INIT(oc, instr) case oc: return std::make_unique<instr>(FPcond, fpRegisterFile[ft], fpRegisterFile[fs])
    if (opcode == FP_TYPE) {
        if (fmt == BC) {
            if (ft) return std::make_unique<FPBranchOnTrue>(programCounter, FPcond, immediate);
            return std::make_unique<FPBranchOnFalse>(programCounter, FPcond, immediate);
        }

        switch (FPFunct(funct)) {
            FR_INIT(FPADD, FPAddSingle);
            FR_INIT(FPDIV, FPDivideSingle);
            FR_INIT(FPMUL, FPMultiplySingle);
            FR_INIT(FPSUB, FPSubtractSingle);
            FCMP_INIT(FPCEQ, FPCompareEqualSingle);
            FCMP_INIT(FPCLT, FPCompareLessThanSingle);
            FCMP_INIT(FPCLE, FPCompareLessThanOrEqualSingle);
            case FPABS: return std::make_unique<FPAbsoluteValueSingle>(fpRegisterFile[fd], fpRegisterFile[fs]);
            default:
                std::cout << "Not implemented yet: " << std::hex << binary_instruction << std::endl;
                throw 3;
        }
    }

    #define I_GEN_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rt], registerFile[rs], immediate)
    #define I_MEM_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rt], registerFile[rs], immediate, RAM)
    #define I_BRANCH_INIT(oc, instr) case oc: return std::make_unique<instr>(registerFile[rt], registerFile[rs], immediate, programCounter)
    #define FPMEM_INIT(oc, instr) case oc: return std::make_unique<instr>(fpRegisterFile[rt], registerFile[rs], immediate, RAM)
    switch (opcode) {
        I_GEN_INIT(ADDI, AddImmediate);
        I_GEN_INIT(ADDIU, AddImmediateUnsigned);
        I_GEN_INIT(ANDI, AndImmediate);
        I_GEN_INIT(SLTI, SetLessThanImmediate);
        I_GEN_INIT(SLTIU, SetLessThanImmediateUnsigned);
        I_GEN_INIT(ORI, OrImmediate);
        I_MEM_INIT(LW, LoadWord);
        I_MEM_INIT(SW, StoreWord);
        I_BRANCH_INIT(BEQ, BranchOnEqual);
        I_BRANCH_INIT(BNE, BranchOnNotEqual);
        FPMEM_INIT(LWC1, LoadFPSingle);
        FPMEM_INIT(SWC1, StoreFPSingle);
        case J: return std::make_unique<Jump>(programCounter, address);
        case JAL: return std::make_unique<JumpAndLink>(programCounter, address, registerFile[RA]);
        case LUI: return std::make_unique<LoadUpperImmediate>(registerFile[rt], immediate);
        default:
            std::cout << "hello from bad opcode " << std::hex << Word(binary_instruction) << std::endl;
            throw 2;
    }

    throw 3;
    return nullptr;
    
}