#ifndef __PROCESS_H__
#define __PROCESS_H__
#include "ASMInterface.h"
#include "kstl/Bitset.h"

namespace kernel {
    
    size_t constexpr MEM_AVAIL = 256 * 1024 * 1024;
    size_t constexpr PAGE_SIZE = 4096;
    size_t constexpr NUM_PAGES = MEM_AVAIL / PAGE_SIZE;

    class MemoryManager {
        ministl::bitset<NUM_PAGES> freePages;
        size_t kernelReservedBoundary;

    public:
        MemoryManager();

        static MemoryManager& instance() {
            static MemoryManager inst;
            return inst;
        }

        size_t reserveFreeFrame();
        size_t freeFrame();
    };

    enum ProcessState : uint32_t {
        READY,
        RUNNING,
        BLOCKED,
        ZOMBIE
    };

    class PCB {
    public:
        RegisterContext regCtx;

    private:
        uint32_t PID;

        PCB();  // private PCB constctr for kernel

    public:
        ProcessState state;

        PCB(const char* binaryFile, bool fromSpim = false);

        static PCB& kernel() {
            static PCB kpcb;
            return kpcb;
        }
    };


}

#endif