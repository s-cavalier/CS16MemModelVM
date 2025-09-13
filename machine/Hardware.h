#ifndef __HARDWARE_H__
#define __HARDWARE_H__
#include "Processors.h"
#include "FileSystem.h"
#include "../ExternalInfo.h"
#include <unordered_map>
#include <array>
#include <string>
#include "LRUCache.h"

// TODO: Implement memory access guards

using DoubleWord = unsigned long long;
using Word = unsigned int;
using HalfWord = unsigned short;
using Byte = unsigned char;

namespace Hardware {
    struct Instruction;

    struct Machine {
        CPU cpu;
        FloatingPointUnit fpu;
        SystemControlUnit scu;
        Memory memory;
        FileSystem fileSystem;
        Word trapEntry;

        Machine();

        void raiseTrap(Byte exceptionCode, Word badAddr);
        bool killed;

        void loadKernel(const ExternalInfo::KernelBootInformation& kernelInfo, const std::vector<std::string>& kernelArguments);

        void step( InterruptDevice* device = nullptr );

        // Just calls a loop on runInstruction until kill flag is set
        using instrDebugHook = void (*)(const Machine&);
        void run(instrDebugHook hook = nullptr, InterruptDevice* device = nullptr, std::chrono::milliseconds IDduration = std::chrono::milliseconds(1) );
    };

    struct Instruction {
        virtual void run() = 0;
    };

};

#endif