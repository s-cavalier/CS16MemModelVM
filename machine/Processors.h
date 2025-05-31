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

    class Coprocessor {
    protected:
        Machine& machine;             // could switch to a bus model later on
        reg32_t registerFile[32];
    
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
    };

    class SystemControlUnit : public Coprocessor {
        
    
    public:
        SystemControlUnit(Machine& machine);

        inline void setEPC(const Word& pc) { registerFile[14].ui = pc; }
        inline void setCause(const Byte& exceptionCode) { registerFile[13].ui = Word(exceptionCode) << 2; }
        inline void setBadVAddr(const Word& vaddr) { registerFile[8].ui = vaddr; }
        inline void setEXL(const bool& enable) { if (enable) registerFile[12].ui |= 0b10; else registerFile[12].ui &= ~0b10; }
        inline bool readEXL() { return registerFile[12].ui & 0b10; }

        std::unique_ptr<Instruction> decode(const Word& bin_instr);

    };

};


#endif