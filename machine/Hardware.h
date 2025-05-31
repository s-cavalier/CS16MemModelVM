#ifndef __HARDWARE_H__
#define __HARDWARE_H__
#include "Processors.h"
#include <unordered_map>
#include <vector>
#include <array>

// TODO: Implement memory access guards

namespace Hardware {

    class Machine {
        bool privlieged;
        std::unordered_map<Word, std::unique_ptr<Instruction>> instructionCache;
        std::array<std::unique_ptr<Coprocessor>, 3> coprocessors;
        Memory RAM;
        CPU cpu;
        
    public:
        Machine();

        const Word& readProgramCounter() const;
        const int& readRegister(const Byte& reg) const;
        const Memory& readMemory() const;
        const float& readFPRegister(const Byte& reg) const;
        bool killed;

        void loadInstructions(const std::vector<Word>& instructions);
        void loadData(const std::vector<Byte>& bytes);

        void runInstruction();

        // Just calls a loop on runInstruction until kill flag is set
        void run();
    };

    struct Instruction {
        virtual void run() = 0;
    };

    // std::unique_ptr<Instruction> instructionFactory(const Word& binary_instruction, Hardware::Machine::RegisterFile& registers, bool& kill_flag);

};

#endif