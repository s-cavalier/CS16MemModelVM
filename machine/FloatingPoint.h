#ifndef __FLOATING_POINT_H__
#define __FLOATING_POINT_H__
#include "SpecialInstruction.h"
#include "IInstruction.h"

// ------------------------------------------------------------
// Floating Point instructions
// Construct with references convert to pointers so no nullptrs
// Use pointers so double precision ops are easier
// ------------------------------------------------------------

class FPInstruction : public Hardware::Instruction {
protected:
    float* ft;
    
public:
    FPInstruction(float& ft);
    virtual void run() = 0;
};

// FPBranch is unique; doesn't need ft, just branch on FPcond - maybe inherit from IInstruction?

#define FBC_INSTR_ARGS Word& pc, bool& FPcond, const short& imm
class FPBranchInstruction : public Hardware::Instruction {
protected:
    Word& pc;
    const bool& FPcond;
    short imm;

public:
    FPBranchInstruction(FBC_INSTR_ARGS);
    virtual void run() = 0;
};

#define FR_INSTR_ARGS float& ft, float& fs, float& fd
class FRInstruction : public FPInstruction {
protected:
    float* fs;
    float* fd;

public:
    FRInstruction(FR_INSTR_ARGS);
    virtual void run() = 0;
};

#define FPCMP_INSTR_ARGS bool& FPcond, float& ft, float& fs
class FPCompareInstruction : public FPInstruction {
protected:
    bool& FPcond;
    float* fs;

public:
    FPCompareInstruction(FPCMP_INSTR_ARGS);
    virtual void run() = 0;
};

// ---------
// FP memory
// ---------
#define FPMEM_INSTR_ARGS float& rt, int& rs, const short& imm, Hardware::Memory& mem
class FPMemoryInstruction : public IInstruction {
protected:
    float* rt;
    int& rs;
    Hardware::Memory& mem;
public:
    FPMemoryInstruction(FPMEM_INSTR_ARGS);
    virtual void run() = 0;
};

//TODO: Add double precision

#define FR_INSTR(x) struct x : public FRInstruction { x(FR_INSTR_ARGS); void run(); }
FR_INSTR(FPAddSingle);
FR_INSTR(FPDivideSingle);
FR_INSTR(FPMultiplySingle);
FR_INSTR(FPSubtractSingle);

#define FCMP_INSTR(x) struct x : public FPCompareInstruction { x(FPCMP_INSTR_ARGS); void run(); }
FCMP_INSTR(FPCompareEqualSingle);
FCMP_INSTR(FPCompareLessThanSingle);
FCMP_INSTR(FPCompareLessThanOrEqualSingle);

#define FBC_INSTR(x) struct x : public FPBranchInstruction { x(FBC_INSTR_ARGS); void run(); }
FBC_INSTR(FPBranchOnTrue);
FBC_INSTR(FPBranchOnFalse);

#define FPMEM_INSTR(x) struct x : public FPMemoryInstruction { x(FPMEM_INSTR_ARGS); void run(); }
FPMEM_INSTR(LoadFPSingle);
FPMEM_INSTR(StoreFPSingle);

// ABS is weird
struct FPAbsoluteValueSingle : public Hardware::Instruction {
protected:
    float& fd;
    float& fs;
public:
    FPAbsoluteValueSingle(float& fd, float& fs);
    void run();
};


#endif