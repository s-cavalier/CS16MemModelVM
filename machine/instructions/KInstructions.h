#include "../Hardware.h"

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