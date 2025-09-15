#ifndef __FLOATING_POINT_H__
#define __FLOATING_POINT_H__
#include "SpecialInstruction.h"
#include "IInstruction.h"
#include <type_traits>

// ------------------------------------------------------------
// Floating Point instructions
// Construct with references convert to pointers so no nullptrs
// Use pointers so double precision ops are easier
// ------------------------------------------------------------

using Single = float;
using Double = double;

template <typename FPType>
class FPInstruction : public Hardware::Instruction {
    // since it's defined in the CPP i don't think its necessary here but may as well
    static_assert(
        std::is_same_v<FPType, Single> || std::is_same_v<FPType, Double>,
        "FPInstructions only can hold float/double."
    );

protected:
    FPType& ft;
    
public:
    FPInstruction(FPType& ft);
    virtual void run() = 0;
};

#define FR_INSTR_ARGS FPType& ft, FPType& fs, FPType& fd
template <typename FPType>
class FRInstruction : public FPInstruction<FPType> {
protected:
    FPType& fs;
    FPType& fd;

public:
    FRInstruction(FR_INSTR_ARGS);
    virtual void run() = 0;
};

#define FR_INSTR(x) template <typename FPType> struct x : public FRInstruction<FPType> { x(FR_INSTR_ARGS); void run(); }
FR_INSTR(FPAdd);
FR_INSTR(FPDivide);
FR_INSTR(FPMultiply);
FR_INSTR(FPSubtract);

// only r-types, plus it won't be more than 4 instr with single double load/stores
#define FPS_MEM_INSTR_ARGS float& rt, int& rs, const short& imm, Hardware::Memory& mem, Hardware::TLB& tlb, Word& asidReg
class FPSingleMemoryInstruction : public IInstruction {
protected:
    float& rt;
    int& rs;
    Hardware::Memory& mem;
    Hardware::TLB& tlb;
    Word& asidReg;

public:
    FPSingleMemoryInstruction(FPS_MEM_INSTR_ARGS);
    virtual void run() = 0;
};

#define FPS_MEM_INSTR(x) struct x : public FPSingleMemoryInstruction { x(FPS_MEM_INSTR_ARGS); void run(); }
FPS_MEM_INSTR(LoadFPSingle);
FPS_MEM_INSTR(StoreFPSingle);

// load in template impl
#include "FloatingPoint.impl.hpp"

#endif