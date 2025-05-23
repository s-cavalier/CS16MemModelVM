#include "FloatingPoint.h"

FPInstruction::FPInstruction(float& ft) : ft(&ft) {}
FRInstruction::FRInstruction(FR_INSTR_ARGS) : FPInstruction(ft), fs(&fs), fd(&fd) {}
FIInstruction::FIInstruction(FII_INSTR_ARGS) : FPInstruction(ft), imm(imm) {}

#define FR_INSTR_INIT(x) x::x(FR_INSTR_ARGS) : FRInstruction(ft, fs, fd) {} void x::run()
FR_INSTR_INIT(FPAddSingle) { *fd = *fs + *ft; }