#ifndef __FLOATING_POINT_H__
#define __FLOATING_POINT_H__
#include "SpecialInstruction.h"

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

#define FR_INSTR_ARGS float& ft, float& fs, float& fd
class FRInstruction : public FPInstruction {
protected:
    float* fs;
    float* fd;

public:
    FRInstruction(FR_INSTR_ARGS);
    virtual void run() = 0;
};

#define FII_INSTR_ARGS float& ft, const Byte& imm
class FIInstruction : public FPInstruction {
protected:
    float imm;

public:
    FIInstruction(FII_INSTR_ARGS);
    virtual void run() = 0;
};

#define FR_INSTR(x) struct x : public FRInstruction { x(FR_INSTR_ARGS); void run(); }
FR_INSTR(FPAddSingle);

#endif