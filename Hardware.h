#ifndef __HARDWARE_H__
#define __HARDWARE_H__
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

using Word = unsigned int;
using HalfWord = unsigned short;
using Byte = unsigned char;

namespace Hardware {
    struct Instruction;

    class Machine {
        Word programCounter;
        int registerFile[32];
        std::unordered_map<Word, int> RAM;
        std::unordered_map<Word, std::unique_ptr<Instruction>> instructionCache;

    public:
        Machine();

        void runInstruction();
    };

    struct Instruction {
        virtual void run() = 0;
    };

};

#endif