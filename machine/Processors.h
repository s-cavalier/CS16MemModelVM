#ifndef __PROCESSORS_H__
#define __PROCESSORS_H__
#include "Memory.h"
#include <memory>
#include <cassert>

using Word = unsigned int;
using HalfWord = unsigned short;
using Byte = unsigned char;
using Double = double;
using Single = float;

namespace Hardware {
    struct Instruction;
    class Machine;

    union reg32_t {
        Word ui;
        int i;
        float f;
    };

    class Coprocessor {
    protected:
        alignas(8) reg32_t registerFile[32];    // aligned for FPU to be able to reinterpret_cast pairs of registers into double, switch to alignas(16) if implementing quad-prec
        Machine& machine;             // could switch to a bus model later on
        
    
    public:
        template <unsigned int N>
        using Array = std::array<std::unique_ptr<Coprocessor>, N>;

        Coprocessor(Machine& machine);

        inline const reg32_t& readRegister(const Byte& reg) const { return registerFile[reg]; }
        inline reg32_t& accessRegister(const Byte& reg) { return registerFile[reg]; }

        virtual std::unique_ptr<Instruction> decode(const Word& bin_instr) = 0;

        virtual ~Coprocessor() = default;
    };

    class FloatingPointUnit : public Coprocessor {
        bool FPcond;

    public:
        FloatingPointUnit(Machine& machine);
        std::unique_ptr<Instruction> decode(const Word& bin_instr);

        inline const Double& getDouble(const Byte& reg) const { 
            using DoubleAlias __attribute__((__may_alias__)) = Double;
            assert(!(reg & 1)); 
            return *reinterpret_cast<const DoubleAlias*>(&registerFile[reg].f); 
        }

        inline Double& getDouble(const Byte& reg) { 
            using DoubleAlias __attribute__((__may_alias__)) = Double;
            assert(!(reg & 1)); 
            return *reinterpret_cast<DoubleAlias*>(&registerFile[reg].f); 
        }
    };

    class SystemControlUnit : public Coprocessor {
    
    public:
        SystemControlUnit(Machine& machine);

        inline void setEPC(const Word& pc) { registerFile[14].ui = pc; }
        inline void setCause(const Byte& exceptionCode) { registerFile[13].ui = Word(exceptionCode) << 2; }
        inline void setBadVAddr(const Word& vaddr) { registerFile[8].ui = vaddr; }
        inline void setEXL(const bool& enable) { if (enable) registerFile[12].ui |= 0b10; else registerFile[12].ui &= ~Word(0b10); }

        std::unique_ptr<Instruction> decode(const Word& bin_instr);

    };

};


#endif