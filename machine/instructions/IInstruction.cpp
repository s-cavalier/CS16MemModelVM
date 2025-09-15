#include "IInstruction.h"

IInstruction::IInstruction(const short& imm) : imm(imm) {}
IGenericInstruction::IGenericInstruction(I_GEN_INSTR_ARGS) : IInstruction(imm), rt(rt), rs(rs) {}
IMemoryInstruction::IMemoryInstruction(I_MEM_INSTR_ARGS) : IGenericInstruction(rt, rs, imm), mem(mem), tlb(tlb) {}
IBranchInstruction::IBranchInstruction(I_BRANCH_INSTR_ARGS) : IGenericInstruction(rt, rs, imm), pc(pc) {}
IBranchZeroInstruction::IBranchZeroInstruction(I_BRANCH_ZERO_INSTR_ARGS) : IInstruction(imm), pc(pc), rs(rs) {}
ISingleInstruction::ISingleInstruction(int& arg0, const short& imm) : IInstruction(imm), arg0(arg0) {}

#define I_GEN_CONSTRCTR_INIT(x) x::x(I_GEN_INSTR_ARGS) : IGenericInstruction(rt, rs, imm) {} void x::run()
I_GEN_CONSTRCTR_INIT(AddImmediate) { if (__builtin_add_overflow(rs, int(imm), &rt)) throw Hardware::Trap(Hardware::Trap::OVERFLOW); }
I_GEN_CONSTRCTR_INIT(AddImmediateUnsigned) { rt = Word(rs) + int(imm); }
I_GEN_CONSTRCTR_INIT(AndImmediate) { rt = rs & int(HalfWord(imm)); }
I_GEN_CONSTRCTR_INIT(OrImmediate) { rt = rs | int(HalfWord(imm)); }
I_GEN_CONSTRCTR_INIT(XorImmediate) { rt = rs ^ int(HalfWord(imm)); }
I_GEN_CONSTRCTR_INIT(SetLessThanImmediate) { rt = (rs < imm ? 1 : 0); }
I_GEN_CONSTRCTR_INIT(SetLessThanImmediateUnsigned) { rt = (Word(rs) < Word(HalfWord(imm)) ? 1 : 0); }

#define I_MEM_CONSTRCTR_INIT(x) x::x(I_MEM_INSTR_ARGS) : IMemoryInstruction(rt, rs, imm, mem, tlb) {} void x::run()
I_MEM_CONSTRCTR_INIT(LoadByte) { rt = char(mem.getByte(Word(rs) + int(imm), tlb)); }
I_MEM_CONSTRCTR_INIT(LoadByteUnsigned) { rt = Word(mem.getByte(Word(rs) + int(imm), tlb)); }
I_MEM_CONSTRCTR_INIT(LoadHalfword) { rt = short(mem.getHalfWord(Word(rs) + int(imm), tlb)); }
I_MEM_CONSTRCTR_INIT(LoadHalfwordUnsigned) { rt = Word(mem.getHalfWord(Word(rs) + int(imm), tlb)); }
I_MEM_CONSTRCTR_INIT(LoadWord) { rt = mem.getWord(Word(rs) + int(imm), tlb); }
I_MEM_CONSTRCTR_INIT(StoreByte) { mem.setByte(Word(rs) + int(imm), Byte(rt), tlb); }
I_MEM_CONSTRCTR_INIT(StoreHalfword) { mem.setHalfWord(Word(rs) + int(imm), HalfWord(rt), tlb); }
I_MEM_CONSTRCTR_INIT(StoreWord) { mem.setWord(Word(rs) + int(imm), rt, tlb); }


#define I_BRANCH_CONSTRCTR_INIT(x) x::x(I_BRANCH_INSTR_ARGS) : IBranchInstruction(rt, rs, imm, pc) {} void x::run()
I_BRANCH_CONSTRCTR_INIT(BranchOnEqual) { if (rt == rs) pc += (int(imm) << 2); }
I_BRANCH_CONSTRCTR_INIT(BranchOnNotEqual) { if (rt != rs) pc += (int(imm) << 2); }

#define I_BRANCH_ZERO_CONSTCTR_INIT(x) x::x(I_BRANCH_ZERO_INSTR_ARGS) : IBranchZeroInstruction(imm, pc, rs) {} void x::run()
I_BRANCH_ZERO_CONSTCTR_INIT(BranchOnGreaterThanOrEqualZero) { if (rs >= 0) pc += (int(imm) << 2); }
I_BRANCH_ZERO_CONSTCTR_INIT(BranchOnGreaterThanZero) { if (rs > 0) pc += (int(imm) << 2); }
I_BRANCH_ZERO_CONSTCTR_INIT(BranchOnLessThanOrEqualZero) { if (rs <= 0) pc += (int(imm) << 2); }
I_BRANCH_ZERO_CONSTCTR_INIT(BranchOnLessThanZero) { if (rs < 0) pc += (int(imm) << 2); }

LoadUpperImmediate::LoadUpperImmediate(int& rt, const short& imm) : ISingleInstruction(rt, imm) {}
void LoadUpperImmediate::run() { arg0 = int(HalfWord(imm)) << 16; }
