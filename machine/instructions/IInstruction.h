#ifndef __IINSTRUCTION_H__
#define __IINSTRUCTION_H__
#include "../Hardware.h"

// ---------------------------------
// I-Type Instructions
// Follows op rt, rs, imm
// --------------------------------

class IInstruction : public Hardware::Instruction {
protected:
    short imm;

public:
    IInstruction(const short& imm);

    virtual void run() = 0;
};

// --------------------------
// Generic I-Type Instruction
// Follows op rt, rs, imm
// --------------------------

#define I_GEN_INSTR_ARGS int& rt, int& rs, const short& imm
class IGenericInstruction : public IInstruction {
protected:
    int& rt;
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
I_GEN_INSTR(XorImmediate);
I_GEN_INSTR(SetLessThanImmediate);
I_GEN_INSTR(SetLessThanImmediateUnsigned);


// ----------------------------------------------
// Memory Instructions
// mem holds reference to specific memory address
// ----------------------------------------------

#define I_MEM_INSTR_ARGS I_GEN_INSTR_ARGS, Hardware::Memory& mem, Hardware::TLB& tlb
class IMemoryInstruction : public IGenericInstruction {
protected:
    Hardware::Memory& mem;
    Hardware::TLB& tlb;

public:
    IMemoryInstruction(I_MEM_INSTR_ARGS);
    virtual void run() = 0;
};

// -------------------------------------------------------------
// Memory Instruction Prototypes
// Maybe split into load/store types later? Probably unnecessary
// -------------------------------------------------------------

#define I_MEM_INSTR(x) struct x : public IMemoryInstruction { x(I_MEM_INSTR_ARGS); void run(); }
I_MEM_INSTR(LoadByte);
I_MEM_INSTR(LoadByteUnsigned);
I_MEM_INSTR(LoadHalfword);
I_MEM_INSTR(LoadHalfwordUnsigned);
I_MEM_INSTR(LoadWord);
I_MEM_INSTR(StoreByte);
I_MEM_INSTR(StoreHalfword);
I_MEM_INSTR(StoreWord);

I_MEM_INSTR(LoadLinked);        // not needed until later
I_MEM_INSTR(StoreConditional);  // until working on OS

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

// --------------
// ZERO branches
// --------------

#define I_BRANCH_ZERO_INSTR_ARGS const short& imm, Word& pc, const int& rs
class IBranchZeroInstruction : public IInstruction {
protected:
    Word& pc;
    const int& rs;

public:
    IBranchZeroInstruction(I_BRANCH_ZERO_INSTR_ARGS);
    virtual void run() = 0;
};

#define I_BRANCH_ZERO_INSTR(x) struct x : public IBranchZeroInstruction { x(I_BRANCH_ZERO_INSTR_ARGS); void run(); }
I_BRANCH_ZERO_INSTR(BranchOnGreaterThanOrEqualZero);
I_BRANCH_ZERO_INSTR(BranchOnGreaterThanZero);
I_BRANCH_ZERO_INSTR(BranchOnLessThanOrEqualZero);
I_BRANCH_ZERO_INSTR(BranchOnLessThanZero);


// --------------------------------------------------------
// Single Argument Immediates
// follows op, reg, imm
// --------------------------------------------------------
class ISingleInstruction : public IInstruction {
protected:
    int& arg0;

public:
    ISingleInstruction(int& arg0, const short& imm);
    virtual void run() = 0;
};

struct LoadUpperImmediate : public ISingleInstruction {
    LoadUpperImmediate(int& rt, const short& imm);
    void run();
};


#endif