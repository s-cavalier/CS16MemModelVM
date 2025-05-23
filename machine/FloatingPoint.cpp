#include "FloatingPoint.h"
#include <cmath>

FPInstruction::FPInstruction(float& ft) : ft(&ft) {}
FRInstruction::FRInstruction(FR_INSTR_ARGS) : FPInstruction(ft), fs(&fs), fd(&fd) {}
FPBranchInstruction::FPBranchInstruction(FBC_INSTR_ARGS) : pc(pc), FPcond(FPcond), imm(imm) {}
FPCompareInstruction::FPCompareInstruction(FPCMP_INSTR_ARGS) : FPInstruction(ft), FPcond(FPcond), fs(&fs) {}
FPMemoryInstruction::FPMemoryInstruction(FPMEM_INSTR_ARGS) : IInstruction(imm), rt(&rt), rs(rs), mem(mem) {}

#define FR_INSTR_INIT(x) x::x(FR_INSTR_ARGS) : FRInstruction(ft, fs, fd) {} void x::run()
FR_INSTR_INIT(FPAddSingle) { *fd = *fs + *ft; }
FR_INSTR_INIT(FPDivideSingle) { *fd = *fs / *ft; }
FR_INSTR_INIT(FPMultiplySingle) { *fd = *fs * *ft; }
FR_INSTR_INIT(FPSubtractSingle) { *fd = *fs - *ft; }

#define FCMP_INSTR_INIT(x) x::x(FPCMP_INSTR_ARGS) : FPCompareInstruction(FPcond, ft, fs) {} void x::run()
FCMP_INSTR_INIT(FPCompareEqualSingle) { FPcond = *fs == *ft; }  // Consider Invalid flag on NaN
FCMP_INSTR_INIT(FPCompareLessThanSingle) { FPcond = *fs < *ft; }
FCMP_INSTR_INIT(FPCompareLessThanOrEqualSingle) { FPcond = *fs <= *ft; }

#define FBC_INSTR_INIT(x) x::x(FBC_INSTR_ARGS) : FPBranchInstruction(pc, FPcond, imm) {} void x::run()
FBC_INSTR_INIT(FPBranchOnTrue) { if (FPcond) pc += (int(imm) << 2) - 4; }
FBC_INSTR_INIT(FPBranchOnFalse) { if (!FPcond) pc += (int(imm) << 2) - 4; }

#define FPMEM_INSTR_INIT(x) x::x(FPMEM_INSTR_ARGS) : FPMemoryInstruction(rt, rs, imm, mem) {} void x::run()
FPMEM_INSTR_INIT(LoadFPSingle) { *rt = mem.getSingle(Word(rs) + int(imm)); }
FPMEM_INSTR_INIT(StoreFPSingle) { mem.setSingle(Word(rs) + int(imm), *rt); }

// ABS is weird
FPAbsoluteValueSingle::FPAbsoluteValueSingle(float& fd, float& fs) : fd(fd), fs(fs) {}
void FPAbsoluteValueSingle::run() { fd = fabsf32(fs); }