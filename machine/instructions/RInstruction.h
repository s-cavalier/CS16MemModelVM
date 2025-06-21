#ifndef __RINSTRUCTION_H__
#define __RINSTRUCTION_H__
#include "../Hardware.h"

// -------------------
// R-Type Instructions
// -------------------
class RInstruction : public Hardware::Instruction {
protected:
    int& rd;
    int& rt; 

public:
    RInstruction(int& rd, int& rt);

    virtual void run() = 0;
};

// -----------------------------------------
// Instructions that follow op rd, rt, shamt
// -----------------------------------------

#define R_SHFT_INSTR_ARGS int& rd, int& rt, const Byte& shamt
class RShiftInstruction : public RInstruction {
protected:
    Byte shamt;

public:
    RShiftInstruction(R_SHFT_INSTR_ARGS);

    virtual void run() = 0;
};

// ---------------------------------------
// Instructions that follow op, rd, rt, rs
// ---------------------------------------

#define R_VAR_INSTR_ARGS int& rd, int& rt, int& rs
class RVariableInstruction : public RInstruction {
protected:
    int& rs;

public:
    RVariableInstruction(R_VAR_INSTR_ARGS);

    virtual void run() = 0;
};

// ---------------------------------
// R Variable Instruction Prototypes
// ---------------------------------
#define R_VAR_INSTR(x) struct x : public RVariableInstruction { x(R_VAR_INSTR_ARGS); void run(); }
R_VAR_INSTR(Add);
R_VAR_INSTR(AddUnsigned);
R_VAR_INSTR(And);
R_VAR_INSTR(Nor);
R_VAR_INSTR(Or);
R_VAR_INSTR(Xor);
R_VAR_INSTR(SetLessThan);
R_VAR_INSTR(SetLessThanUnsigned);
R_VAR_INSTR(Subtract);
R_VAR_INSTR(SubtractUnsigned);
R_VAR_INSTR(ShiftLeftLogicalVariable);
R_VAR_INSTR(ShiftRightLogicalVariable);
R_VAR_INSTR(ShiftRightArithmeticVariable);
R_VAR_INSTR(MoveOnNotZero);

// ------------------------------
// R Shift Instruction Prototypes
// ------------------------------
#define R_SHFT_INSTR(x) struct x : public RShiftInstruction { x(R_SHFT_INSTR_ARGS); void run(); }
R_SHFT_INSTR(ShiftLeftLogical);
R_SHFT_INSTR(ShiftRightLogical);
R_SHFT_INSTR(ShiftRightArithmetic);

#endif