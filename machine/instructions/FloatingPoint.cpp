#include "FloatingPoint.h"

FPSingleMemoryInstruction::FPSingleMemoryInstruction(FPS_MEM_INSTR_ARGS) : IInstruction(imm), rt(rt), rs(rs), mem(mem) {}
#define FPS_MEM_INSTR_INIT(x) x::x(FPS_MEM_INSTR_ARGS) : FPSingleMemoryInstruction(rt, rs, imm, mem) {} void x::run()
FPS_MEM_INSTR_INIT(LoadFPSingle) { rt = mem.getSingle(Word(rs) + int(imm)); }
FPS_MEM_INSTR_INIT(StoreFPSingle) { mem.setSingle(Word(rs) + int(imm), rt); }