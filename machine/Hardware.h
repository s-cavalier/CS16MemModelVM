#ifndef __HARDWARE_H__
#define __HARDWARE_H__
#include "Memory.h"
#include <unordered_map>
#include <memory>
#include <vector>

// TODO: Implement memory access guards

using DoubleWord = unsigned long long;
using Word = unsigned int;
using HalfWord = unsigned short;
using Byte = unsigned char;

namespace Hardware {
    struct Instruction;

    class Machine {
    public:
        struct HiLoRegisters { Word hi; Word lo; }; // If a system is big endian, speedups could be done here?
        struct RegisterFile {
            Word programCounter;
            int registerFile[32];       // Maybe use a union between int/unsigned int? Probnot
            float fpRegisterFile[32];
            HiLoRegisters hiLo;
            Memory RAM;
            bool FPcond;
        };

    private:
        RegisterFile registers;
        std::unordered_map<Word, std::unique_ptr<Instruction>> instructionCache;
        
    public:
        Machine();

        const Word& readProgramCounter() const;
        const int& readRegister(const Byte& reg) const;
        const Memory& readMemory() const;
        const float& readFPRegister(const Byte& reg) const;
        bool killed;

        void loadInstructions(const std::vector<Word>& instructions);

        void runInstruction();

        // Just calls a loop on runInstruction until kill flag is set
        void run();
    };

    struct Instruction {
        virtual void run() = 0;
    };

    std::unique_ptr<Instruction> instructionFactory(const Word& binary_instruction, Hardware::Machine::RegisterFile& registers, bool& kill_flag);

};

#endif