#include "FloatingPoint.h"

FPSingleMemoryInstruction::FPSingleMemoryInstruction(FPS_MEM_INSTR_ARGS) : IInstruction(imm), rt(rt), rs(rs), mem(mem), tlb(tlb) {}
#define FPS_MEM_INSTR_INIT(x) x::x(FPS_MEM_INSTR_ARGS) : FPSingleMemoryInstruction(rt, rs, imm, mem, tlb) {} void x::run()
FPS_MEM_INSTR_INIT(LoadFPSingle) { rt = mem.getSingle(Word(rs) + int(imm), tlb); }
FPS_MEM_INSTR_INIT(StoreFPSingle) { mem.setSingle(Word(rs) + int(imm), rt, tlb); }