#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__
#include "Hardware.h"
#include <memory>

// --------------------------------------------------------------------------
// Instruction definitions and utilities
// Register references reference the actual register instead of holding value
// Macros exist for cleaner code
// --------------------------------------------------------------------------

std::unique_ptr<Hardware::Instruction> instructionFactory(const Word& binary_instruction, Word& programCounter, int* registerFile, Hardware::Memory& RAM, bool& kill_flag);

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
R_VAR_INSTR(SetLessThan);
R_VAR_INSTR(SetLessThanUnsigned);
R_VAR_INSTR(Subtract);
R_VAR_INSTR(SubtractUnsigned);

// ------------------------------
// R Shift Instruction Prototypes
// ------------------------------
#define R_SHFT_INSTR(x) struct x : public RShiftInstruction { x(R_SHFT_INSTR_ARGS); void run(); }
R_SHFT_INSTR(ShiftLeftLogical);
R_SHFT_INSTR(ShiftRightLogical);

// -------------------
// I-Type Instructions
// -------------------

class IInstruction : public Hardware::Instruction {
protected:
    int& rt;
    short imm;

public:
    IInstruction(int& rt, const short& imm);

    virtual void run() = 0;
};

// --------------------------------------------------------
// LUI is special, needs its own class (only two inputs)
// --------------------------------------------------------

struct LoadUpperImmediate : public IInstruction {
    LoadUpperImmediate(int& rt, const short& imm);

    void run();
};

// --------------------------
// Generic I-Type Instruction
// Follows op rt, rs, imm
// --------------------------

#define I_GEN_INSTR_ARGS int& rt, int& rs, const short& imm
class IGenericInstruction : public IInstruction {
protected:
    int& rs;

public:
    IGenericInstruction(I_GEN_INSTR_ARGS);

    virtual void run() = 0;
};

// -------------------------------------
// Generic I-Type Instruction Prototypes
// -------------------------------------

#define I_GEN_INSTR(x) struct x : public IGenericInstruction { x(I_GEN_INSTR_ARGS); void run(); }
I_GEN_INSTR(AddImmediate);
I_GEN_INSTR(AddImmediateUnsigned);
I_GEN_INSTR(AndImmediate);
I_GEN_INSTR(OrImmediate);
I_GEN_INSTR(SetLessThanImmediate);
I_GEN_INSTR(SetLessThanImmediateUnsigned);

// ----------------------------------------------
// Memory Instructions
// mem holds reference to specific memory address
// ----------------------------------------------

#define I_MEM_INSTR_ARGS I_GEN_INSTR_ARGS, Hardware::Memory& mem
class IMemoryInstruction : public IGenericInstruction {
protected:
    Hardware::Memory& mem;

public:
    IMemoryInstruction(I_MEM_INSTR_ARGS);

    virtual void run() = 0;
};

// -------------------------------------------------------------
// Memory Instruction Prototypes
// Maybe split into load/store types later? Probably unnecessary
// -------------------------------------------------------------

#define I_MEM_INSTR(x) struct x : public IMemoryInstruction { x(I_MEM_INSTR_ARGS); void run(); }
I_MEM_INSTR(LoadByteUnsigned);
I_MEM_INSTR(LoadHalfwordUnsigned);
I_MEM_INSTR(LoadLinked);
I_MEM_INSTR(LoadWord);
I_MEM_INSTR(StoreByte);
I_MEM_INSTR(StoreConditional);
I_MEM_INSTR(StoreHalfword);
I_MEM_INSTR(StoreWord);

// -------------------
// Branch Instructions
// -------------------

#define I_BRANCH_INSTR_ARGS I_GEN_INSTR_ARGS, Word& pc
class IBranchInstruction : public IGenericInstruction {
protected:
    Word& pc;

public:
    IBranchInstruction(I_BRANCH_INSTR_ARGS);

    virtual void run() = 0;
};

// -----------------------------
// Branch Instruction Prototypes
// -----------------------------

#define I_BRANCH_INSTR(x) struct x : public IBranchInstruction { x(I_BRANCH_INSTR_ARGS); void run(); }
I_BRANCH_INSTR(BranchOnEqual);
I_BRANCH_INSTR(BranchOnNotEqual);

// -----------------
// Jump Instructions
// -----------------

#define J_INSTR_ARGS Word& pc
class JInstruction : public Hardware::Instruction {
protected:
    Word& pc;

public:
    JInstruction(J_INSTR_ARGS);

    virtual void run() = 0;
};

// ------------------------------------------------------------------------
// Jump Instruction Prototypes
// Each branch instr is pretty different, so each once gets their own class
// ------------------------------------------------------------------------

class Jump : public JInstruction {
    Word target;

public:
    Jump(J_INSTR_ARGS, const Word& target);
    void run();
};

class JumpAndLink : public JInstruction {
    Word target;
    int& ra;

public:
    JumpAndLink(J_INSTR_ARGS, const Word& target, int& ra);
    void run();
};

class JumpRegister : public JInstruction {
    const int& ra;

public:
    JumpRegister(J_INSTR_ARGS, const int& ra);
    void run();
};

// -------
// Syscall
// -------

class Syscall : public Hardware::Instruction {
    int& v0;
    int& a0;
    int& a1;
    bool& kill_flag;

public:
    Syscall(int& v0, int& a0, int& a1, bool& kill_flag);
    void run();
};

// -----
// No-Op
// -----
struct NoOp : public Hardware::Instruction {
    void run() {}
};


#endif