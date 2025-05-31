#ifndef __PROCESSORS_H__
#define __PROCESSORS_H__
#include "Memory.h"
#include <memory>

using Word = unsigned int;
using HalfWord = unsigned short;
using Byte = unsigned char;

namespace Hardware {
    struct Instruction;
    class Machine;

    union reg32_t {
        Word ui;
        int i;
        float f;
    };

    struct HiLoRegisters { Word hi; Word lo; };

    class CPU {
        Machine& machine;
        reg32_t registerFile[32];
        HiLoRegisters hiLo;
        Word programCounter;
    
    public:
        CPU(Machine& machine);       // maybe cache later down the road
        inline const reg32_t& readRegister(const Byte& reg) const;

        std::unique_ptr<Instruction> decode(const Word& bin_instr, const Word& programCounter);

    };

    class Coprocessor {
    protected:
        Machine& machine;             // could switch to a bus model later on
        reg32_t registerFile[32];
    
    public:
        Coprocessor(Machine& RAM);

        inline const reg32_t& readRegister(const Byte& reg) const;
        inline void moveToThis(const Byte& regDst, const reg32_t& valueSrc);

        virtual std::unique_ptr<Instruction> decode(const Word& bin_instr, const Word& programCounter) = 0;

        virtual ~Coprocessor() = default;
    };

    class CP1 : public Coprocessor {
        bool FPcond;
    
    public:
        CP1(Machine& RAM);

        std::unique_ptr<Instruction> decode(const Word& bin_instr, const Word& programCounter);
    };

};


#endif