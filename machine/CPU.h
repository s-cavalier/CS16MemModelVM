#ifndef __CPU_H__
#define __CPU_H__
#include "Processors.h"

namespace Hardware {

    struct HiLoRegisters { Word hi; Word lo; };

    class CPU {
        Machine& machine;
        reg32_t registerFile[32];
        HiLoRegisters hiLo;
        Word programCounter;
        std::unordered_map<Word, std::unique_ptr<Instruction>> instructionCache;
    
    public:
        CPU(Machine& machine);       // maybe cache later down the road
        inline const reg32_t& readRegister(const Byte& reg) const { return registerFile[reg]; }
        inline reg32_t& accessRegister(const Byte& reg) { return registerFile[reg]; }
        inline const Word& readProgramCounter() const { return programCounter; }
        inline Word& accessProgramCounter() { return programCounter; }

        void cycle();

        std::unique_ptr<Instruction> decode(const Word& bin_instr);

    };
}

#endif