#include "RInstruction.h"

RInstruction::RInstruction(int& rd, int& rt) : rd(rd), rt(rt) {};
RVariableInstruction::RVariableInstruction(R_VAR_INSTR_ARGS) : RInstruction(rd, rt), rs(rs) {}
RShiftInstruction::RShiftInstruction(R_SHFT_INSTR_ARGS) : RInstruction(rd, rt), shamt(shamt) {}

#define R_VAR_CONSTRCTR_INIT(x) x::x(R_VAR_INSTR_ARGS) : RVariableInstruction(rd, rt, rs) {} void x::run()
R_VAR_CONSTRCTR_INIT(Add) { rd = rs + rt; }                        // TODO: Add exception handling

R_VAR_CONSTRCTR_INIT(AddUnsigned) { rd = Word(rs) + Word(rt); }

R_VAR_CONSTRCTR_INIT(And) { rd = rs & rt; }

R_VAR_CONSTRCTR_INIT(Nor) { rd = ~(rs | rt); }

R_VAR_CONSTRCTR_INIT(Or) { rd = rs | rt; }

R_VAR_CONSTRCTR_INIT(SetLessThan) { rd = (rs < rt ? 1 : 0); }

R_VAR_CONSTRCTR_INIT(SetLessThanUnsigned) { rd = ( Word(rs) < Word(rt) ? 1 : 0 ); }

R_VAR_CONSTRCTR_INIT(Subtract) { rd = rs - rt; }

R_VAR_CONSTRCTR_INIT(SubtractUnsigned) { rd = Word(rs) - Word(rt); }

#define R_SHFT_CONSTRCTR_INIT(x) x::x(R_SHFT_INSTR_ARGS) : RShiftInstruction(rd, rt, shamt) {} void x::run()
R_SHFT_CONSTRCTR_INIT(ShiftLeftLogical) { rd = rt << shamt; }

R_SHFT_CONSTRCTR_INIT(ShiftRightLogical) { rd = rt >> shamt; }