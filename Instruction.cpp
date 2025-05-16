#include "Instruction.h"
#include "BinaryInstruction.h"
#include <iostream>
#include <iomanip>

// TODO: Add better error handling

std::unique_ptr<Hardware::Instruction> instructionFactory(const Word& binary_instruction, Word& programCounter, int* registerFile, Hardware::Memory& RAM, bool& kill_flag) {
    // DECODE
    
    Opcode opcode = Opcode((binary_instruction >> 26) & 0b111111);  // For All

    Word address = binary_instruction & 0x1FFFFFF;                  // For Jump

    Register rs = Register((binary_instruction >> 21) & 0b11111);   // For I/R
    Register rt = Register((binary_instruction >> 16) & 0b11111);   // For I/R


    Register rd = Register((binary_instruction >> 11) & 0b11111);   // For R
    Byte shamt = (binary_instruction >> 6) & 0b11111;               // For R
    Funct funct = Funct(binary_instruction & 0b111111);             // For R

    short immediate = binary_instruction & 0xFFFF;                  // For I

    std::cout << "READING INSTRUCTION " << std::hex << std::setw(8) << std::setfill('0') << binary_instruction << std::endl;

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
                return std::make_unique<Syscall>(registerFile[V0], registerFile[A0], registerFile[A1], kill_flag);
            default:
                throw 1;
        }
    }

    // TODO: Figure out how address works for J/JAL
    // TODO: Make offset work for branch
    // TODO: More memory instr

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
        case LUI:
            return std::make_unique<LoadUpperImmediate>(registerFile[rt], immediate);
        default:
            throw 2;
    }

    throw 3;
    return nullptr;
}

RInstruction::RInstruction(int& rd, int& rt) : rd(rd), rt(rt) {};
RVariableInstruction::RVariableInstruction(R_VAR_INSTR_ARGS) : RInstruction(rd, rt), rs(rs) {}
RShiftInstruction::RShiftInstruction(R_SHFT_INSTR_ARGS) : RInstruction(rd, rt), shamt(shamt) {}

IInstruction::IInstruction(int& rt, const short& imm) : rt(rt), imm(imm) {}
IGenericInstruction::IGenericInstruction(I_GEN_INSTR_ARGS) : IInstruction(rt, imm), rs(rs) {}
IMemoryInstruction::IMemoryInstruction(I_MEM_INSTR_ARGS) : IGenericInstruction(rt, rs, imm), mem(mem) {}
IBranchInstruction::IBranchInstruction(I_BRANCH_INSTR_ARGS) : IGenericInstruction(rt, rs, imm), pc(pc) {}

JInstruction::JInstruction(J_INSTR_ARGS) : pc(pc) {}

#define RUN_INSTR(x) void x::run()

#define R_VAR_CONSTRCTR_INIT(x) x::x(R_VAR_INSTR_ARGS) : RVariableInstruction(rd, rt, rs) {}
R_VAR_CONSTRCTR_INIT(Add)
RUN_INSTR(Add) { rd = rt + rs; }                        // TODO: Add Overflow exception handling

R_VAR_CONSTRCTR_INIT(AddUnsigned)
RUN_INSTR(AddUnsigned) { rd = Word(rt) + Word(rs); }

R_VAR_CONSTRCTR_INIT(And)
RUN_INSTR(And) { rd = rt & rs; }

R_VAR_CONSTRCTR_INIT(Nor)
RUN_INSTR(Nor) { rd = ~(rt | rs); }

R_VAR_CONSTRCTR_INIT(Or)
RUN_INSTR(Or) { rd = rt | rs; }

R_VAR_CONSTRCTR_INIT(SetLessThan)
RUN_INSTR(SetLessThan) { rd = (rt < rs ? 1 : 0); }

R_VAR_CONSTRCTR_INIT(SetLessThanUnsigned)
RUN_INSTR(SetLessThanUnsigned) { rd = ( Word(rt) < Word(rs) ? 1 : 0 ); }

R_VAR_CONSTRCTR_INIT(Subtract)
RUN_INSTR(Subtract) { rd = rt - rs; }

R_VAR_CONSTRCTR_INIT(SubtractUnsigned)
RUN_INSTR(SubtractUnsigned) { rd = Word(rt) - Word(rs); }


#define R_SHFT_CONSTRCTR_INIT(x) x::x(R_SHFT_INSTR_ARGS) : RShiftInstruction(rd, rt, shamt) {}
R_SHFT_CONSTRCTR_INIT(ShiftLeftLogical)
RUN_INSTR(ShiftLeftLogical) { rd = rt << shamt; }

R_SHFT_CONSTRCTR_INIT(ShiftRightLogical)
RUN_INSTR(ShiftRightLogical) { rd = rt >> shamt; }


#define I_GEN_CONSTRCTR_INIT(x) x::x(I_GEN_INSTR_ARGS) : IGenericInstruction(rt, rs, imm) {}
I_GEN_CONSTRCTR_INIT(AddImmediate)
RUN_INSTR(AddImmediate) { rt = rs + int(imm); }

I_GEN_CONSTRCTR_INIT(AddImmediateUnsigned)
RUN_INSTR(AddImmediateUnsigned) { rt = Word(rs) + Word(imm); }

I_GEN_CONSTRCTR_INIT(AndImmediate)
RUN_INSTR(AndImmediate) { rt = rs & int(HalfWord(imm)); }

I_GEN_CONSTRCTR_INIT(OrImmediate)
RUN_INSTR(OrImmediate) { rt = rs | int(HalfWord(imm)); }

I_GEN_CONSTRCTR_INIT(SetLessThanImmediate)
RUN_INSTR(SetLessThanImmediate) { rt = (rs < imm ? 1 : 0); }

I_GEN_CONSTRCTR_INIT(SetLessThanImmediateUnsigned)
RUN_INSTR(SetLessThanImmediateUnsigned) { rt = (Word(rs) < Word(HalfWord(imm)) ? 1 : 0); }


#define I_MEM_CONSTRCTR_INIT(x) x::x(I_MEM_INSTR_ARGS) : IMemoryInstruction(rt, rs, imm, mem) {}
I_MEM_CONSTRCTR_INIT(LoadWord)
RUN_INSTR(LoadWord) { rt = mem.getWord(rs + int(imm)); }

I_MEM_CONSTRCTR_INIT(StoreWord)
RUN_INSTR(StoreWord) { mem.setWord(rs + int(imm), rt); }


#define I_BRANCH_CONSTRCTR_INIT(x) x::x(I_BRANCH_INSTR_ARGS) : IBranchInstruction(rt, rs, imm, pc) {}
I_BRANCH_CONSTRCTR_INIT(BranchOnEqual)
RUN_INSTR(BranchOnEqual) { if (rt == rs) pc += int(imm) << 2; }

I_BRANCH_CONSTRCTR_INIT(BranchOnNotEqual)
RUN_INSTR(BranchOnNotEqual) { if (rt != rs) pc += int(imm) << 2; }

LoadUpperImmediate::LoadUpperImmediate(int& rt, const short& imm) : IInstruction(rt, imm) {}
void LoadUpperImmediate::run() { rt = int(HalfWord(imm)) << 16; }

JumpRegister::JumpRegister(J_INSTR_ARGS, const int& ra) : JInstruction(pc), ra(ra) {}
void JumpRegister::run() { pc = ra; }

Syscall::Syscall(int& v0, int& a0, int& a1, bool& kill_flag) : v0(v0), a0(a0), a1(a1), kill_flag(kill_flag) {}
void Syscall::run() {
    switch (v0) {
        case 1:         // Print integer
            std::cout << a0;
            return;
        case 10:        // exit
            kill_flag = true;  
            return;
        default:
            std::cout << "hello from bad syscall" << v0 << std::endl;
            throw 4;
    }
}