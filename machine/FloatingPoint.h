#ifndef __FLOATING_POINT_H__
#define __FLOATING_POINT_H__
#include "SpecialInstruction.h"

class FPInstruction : public Hardware::Instruction {
protected:
    Byte fmt;
    Byte& ft;
public:
    FPInstruction();
    virtual void run() = 0;
};


#endif