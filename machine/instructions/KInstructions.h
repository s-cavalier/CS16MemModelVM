#include "../Hardware.h"
#include "../Memory.h"

// -------------------
// Kernel Instructions
// -------------------
// Can only be used in privilieged mode

#define K_INSTR_ARGS const Word& statusRegister
class KInstruction : public Hardware::Instruction {
protected:
    const Word& statusRegister;

public:
    KInstruction(K_INSTR_ARGS);
    inline bool checkEXL() const { return (statusRegister & 0b10); }
    virtual void run() = 0;

};

// CPR[r0, rd, sel] ← rt
// rd is written to 
class MoveToCoprocessor0 : public KInstruction {
    const int& rt;
    int& rd;

public:
    MoveToCoprocessor0(K_INSTR_ARGS, const int& rt, int& rd);
    void run();
};

// rt ← CPR[0,rd,sel]
// rt is written to 
class MoveFromCoprocessor0 : public KInstruction {
    int& rt;
    const int& rd;

public:
    MoveFromCoprocessor0(K_INSTR_ARGS, int& rt, const int& rd);
    void run();
};

class ExceptionReturn : public KInstruction {
    Hardware::Machine& machine;

public:
    ExceptionReturn(Hardware::Machine& machine);
    void run();
};

// TLB Instructions

#define TLB_INSTR_ARGS K_INSTR_ARGS, Hardware::TLB& tlb

class TLBInstruction : public KInstruction {
protected:
    Hardware::TLB& tlb;

public:
    TLBInstruction(TLB_INSTR_ARGS);
    virtual void run() = 0;
};

class TLBProbe : public TLBInstruction {
    Word& indexRegister;
    Word& entryHiRegister;

public:
    TLBProbe(TLB_INSTR_ARGS, Word& indexRegister, Word& entryHiRegister);
    void run();
};

class TLBReadIndexed : public TLBInstruction {
    const Word& indexRegister;
    Word& entryHiRegister;
    Word& entryLoRegister;

public:
    TLBReadIndexed(TLB_INSTR_ARGS, const Word& indexRegister, Word& entryHiRegister, Word& entryLoRegister);
    void run();
};

class TLBWriteIndexed : public TLBInstruction {
protected:
    Word& indexRegister;
    Word& entryHiRegister;
    Word& entryLoRegister;

public:
    TLBWriteIndexed(TLB_INSTR_ARGS, Word& indexRegister, Word& entryHiRegister, Word& entryLoRegister);
    void run();
};

struct TLBWriteRandom : public TLBWriteIndexed {
    TLBWriteRandom(TLB_INSTR_ARGS, Word& indexRegister, Word& entryHiRegister, Word& entryLoRegister);
    void run();
};

// ----------------------------------------------------------
// Custom Instructions
// Temporary interface until more is built
// ----------------------------------------------------------

class VMTunnel : public KInstruction {
    Hardware::Machine& machine;

public:
    VMTunnel(K_INSTR_ARGS, Hardware::Machine& machine);
    void run();
};