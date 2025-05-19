#include "IInstruction.h"

IInstruction::IInstruction(const short& imm) : imm(imm) {}
IGenericInstruction::IGenericInstruction(I_GEN_INSTR_ARGS) : IInstruction(imm), rt(rt), rs(rs) {}
IMemoryInstruction::IMemoryInstruction(I_MEM_INSTR_ARGS) : IGenericInstruction(rt, rs, imm), mem(mem) {}
IBranchInstruction::IBranchInstruction(I_BRANCH_INSTR_ARGS) : IGenericInstruction(rt, rs, imm), pc(pc) {}

#define I_GEN_CONSTRCTR_INIT(x) x::x(I_GEN_INSTR_ARGS) : IGenericInstruction(rt, rs, imm) {} void x::run()
I_GEN_CONSTRCTR_INIT(AddImmediate) { rt = rs + int(imm); }
I_GEN_CONSTRCTR_INIT(AddImmediateUnsigned) { rt = Word(rs) + int(imm); }
I_GEN_CONSTRCTR_INIT(AndImmediate) { rt = rs & int(HalfWord(imm)); }
I_GEN_CONSTRCTR_INIT(OrImmediate) { rt = rs | int(HalfWord(imm)); }
I_GEN_CONSTRCTR_INIT(SetLessThanImmediate) { rt = (rs < imm ? 1 : 0); }
I_GEN_CONSTRCTR_INIT(SetLessThanImmediateUnsigned) { rt = (Word(rs) < Word(HalfWord(imm)) ? 1 : 0); }

#define I_MEM_CONSTRCTR_INIT(x) x::x(I_MEM_INSTR_ARGS) : IMemoryInstruction(rt, rs, imm, mem) {} void x::run()
I_MEM_CONSTRCTR_INIT(LoadWord) { rt = mem.getWord(rs + int(imm)); }
I_MEM_CONSTRCTR_INIT(StoreWord) { mem.setWord(rs + int(imm), rt); }


#define I_BRANCH_CONSTRCTR_INIT(x) x::x(I_BRANCH_INSTR_ARGS) : IBranchInstruction(rt, rs, imm, pc) {} void x::run()
I_BRANCH_CONSTRCTR_INIT(BranchOnEqual) { if (rt == rs) pc += (int(imm) << 2) - 4; }
I_BRANCH_CONSTRCTR_INIT(BranchOnNotEqual) { if (rt != rs) pc += (int(imm) << 2) - 4; }

LoadUpperImmediate::LoadUpperImmediate(int& rt, const short& imm) : IInstruction(imm), rt(rt) {}
void LoadUpperImmediate::run() { rt = int(HalfWord(imm)) << 16; }