#ifndef __HARDWARE_H__
#define __HARDWARE_H__
#include "Processors.h"
#include "FileSystem.h"
#include "../ExternalInfo.h"
#include <string>
#include "LRUCache.h"

// TODO: Implement memory access guards

using DoubleWord = unsigned long long;
using Word = unsigned int;
using HalfWord = unsigned short;
using Byte = unsigned char;

namespace Hardware {
    
    struct Machine {
        Core cpu;
        Memory memory;
        FileSystem fileSystem;
        Word trapEntry;

        Machine();

        bool killed;

        void loadKernel(const ExternalInfo::KernelBootInformation& kernelInfo, const std::vector<std::string>& kernelArguments);

        void step();

        // Just calls a loop on runInstruction until kill flag is set
        using instrDebugHook = void (*)(const Machine&);
        void run(instrDebugHook hook = nullptr, std::chrono::milliseconds IDduration = std::chrono::milliseconds(1) );
    };

    struct Instruction {
        virtual void run() = 0;
    };

};

#endif