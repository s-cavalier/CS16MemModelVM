#include "IInstruction.h"

IInstruction::IInstruction(const short& imm) : imm(imm) {}
IGenericInstruction::IGenericInstruction(I_GEN_INSTR_ARGS) : IInstruction(imm), rt(rt), rs(rs) {}
IMemoryInstruction::IMemoryInstruction(I_MEM_INSTR_ARGS) : IGenericInstruction(rt, rs, imm), mem(mem) {}
IBranchInstruction::IBranchInstruction(I_BRANCH_INSTR_ARGS) : IGenericInstruction(rt, rs, imm), pc(pc) {}
ISingleInstruction::ISingleInstruction(int& arg0, const short& imm) : IInstruction(imm), arg0(arg0) {}

#define I_GEN_CONSTRCTR_INIT(x) x::x(I_GEN_INSTR_ARGS) : IGenericInstruction(rt, rs, imm) {} void x::run()
I_GEN_CONSTRCTR_INIT(AddImmediate) { rt = rs + int(imm); }
I_GEN_CONSTRCTR_INIT(AddImmediateUnsigned) { rt = Word(rs) + int(imm); }
I_GEN_CONSTRCTR_INIT(AndImmediate) { rt = rs & int(HalfWord(imm)); }
I_GEN_CONSTRCTR_INIT(OrImmediate) { rt = rs | int(HalfWord(imm)); }
I_GEN_CONSTRCTR_INIT(XorImmediate) { rt = rs ^ int(HalfWord(imm)); }
I_GEN_CONSTRCTR_INIT(SetLessThanImmediate) { rt = (rs < imm ? 1 : 0); }
I_GEN_CONSTRCTR_INIT(SetLessThanImmediateUnsigned) { rt = (Word(rs) < Word(HalfWord(imm)) ? 1 : 0); }

#define I_MEM_CONSTRCTR_INIT(x) x::x(I_MEM_INSTR_ARGS) : IMemoryInstruction(rt, rs, imm, mem) {} void x::run()
I_MEM_CONSTRCTR_INIT(LoadByte) { rt = char(mem.getByte(rs + int(imm))); }
I_MEM_CONSTRCTR_INIT(LoadByteUnsigned) { rt = Word(mem.getByte(rs + int(imm))); }
I_MEM_CONSTRCTR_INIT(LoadHalfword) { rt = short(mem.getHalfWord(rs + int(imm))); }
I_MEM_CONSTRCTR_INIT(LoadHalfwordUnsigned) { rt = Word(mem.getHalfWord(rs + int(imm))); }
I_MEM_CONSTRCTR_INIT(LoadWord) { rt = mem.getWord(rs + int(imm)); }
I_MEM_CONSTRCTR_INIT(StoreByte) { mem.setByte(rs + int(imm), Byte(rt)); }
I_MEM_CONSTRCTR_INIT(StoreHalfword) { mem.setHalfWord(rs + int(imm), HalfWord(rt)); }
I_MEM_CONSTRCTR_INIT(StoreWord) { mem.setWord(rs + int(imm), rt); }


#define I_BRANCH_CONSTRCTR_INIT(x) x::x(I_BRANCH_INSTR_ARGS) : IBranchInstruction(rt, rs, imm, pc) {} void x::run()
I_BRANCH_CONSTRCTR_INIT(BranchOnEqual) { if (rt == rs) pc += (int(imm) << 2) - 4; }
I_BRANCH_CONSTRCTR_INIT(BranchOnNotEqual) { if (rt != rs) pc += (int(imm) << 2) - 4; }

LoadUpperImmediate::LoadUpperImmediate(int& rt, const short& imm) : ISingleInstruction(rt, imm) {}
void LoadUpperImmediate::run() { arg0 = int(HalfWord(imm)) << 16; }

BranchOnLessThanZero::BranchOnLessThanZero(int& rs, const short& imm, Word& pc) : ISingleInstruction(rs, imm), pc(pc) {}
void BranchOnLessThanZero::run() { if (arg0 < 0) pc += (int(imm) << 2) - 4; }

BranchOnGreaterThanZero::BranchOnGreaterThanZero(int& rs, const short& imm, Word& pc) : ISingleInstruction(rs, imm), pc(pc) {}
void BranchOnGreaterThanZero::run() { if (arg0 > 0) pc += (int(imm) << 2) - 4; }