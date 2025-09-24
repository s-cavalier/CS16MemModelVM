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

    // Used to redirect stdin/stdout
    struct stdIODevice {  

        virtual void write(const std::string& data) = 0;
        virtual size_t read(char* buf, size_t bytes) = 0;

        virtual ~stdIODevice() = default;
    };

    struct Terminal : public stdIODevice {

        void write(const std::string& data) override;
        size_t read(char* buf, size_t bytes) override;

    };
    
    struct Machine {
        Core cpu;
        Memory memory;
        FileSystem fileSystem;
        std::unique_ptr<stdIODevice> stdio;
        Word trapEntry;
        

        Machine( std::unique_ptr<stdIODevice> dev = nullptr );

        bool killed;

        void loadKernel(const ExternalInfo::KernelBootInformation& kernelInfo, const std::vector<std::string>& kernelArguments);
        void loadKernel( const std::vector<std::string>& kernelArguments );

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