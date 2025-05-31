#ifndef __HARDWARE_H__
#define __HARDWARE_H__
#include "CPU.h"
#include <unordered_map>
#include <vector>
#include <array>

// TODO: Implement memory access guards

using DoubleWord = unsigned long long;
using Word = unsigned int;
using HalfWord = unsigned short;
using Byte = unsigned char;

namespace Hardware {
    struct Instruction;

    class Machine {
        CPU cpu;
        Memory RAM;
        Coprocessor::Array<1> coprocessors;
        std::unordered_map<Word, std::unique_ptr<Instruction>> instructionCache;
        
    public:
        Machine();

        const Memory& readMemory() const { return RAM; }
        Memory& accessMemory() { return RAM; }
        inline const std::unique_ptr<Coprocessor>& readCoprocessor(const Byte& cp) const { return coprocessors.at(cp); }
        inline std::unique_ptr<Coprocessor>& accessCoprocessor(const Byte& cp) { return coprocessors.at(cp); }
        inline const CPU& readCPU() const { return cpu; }
        inline CPU& accessCPU() { return cpu; }
        bool killed;

        void loadInstructions(const std::vector<Word>& instructions);
        void loadData(const std::vector<Byte>& bytes);

        void step();

        // Just calls a loop on runInstruction until kill flag is set
        using instrDebugHook = void (*)(const Machine& machine);
        void run(instrDebugHook hook = nullptr);
    };

    struct Instruction {
        virtual void run() = 0;
    };

};

#endif