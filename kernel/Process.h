#ifndef __PROCESS_H__
#define __PROCESS_H__
#include "ASMInterface.h"
#include "VirtualMemory.h"

namespace kernel {

    enum ProcessState : uint32_t {
        READY,
        RUNNING,
        BLOCKED,
        ZOMBIE
    };

    class PCB {
    public:
        RegisterContext regCtx;
        AddressSpace addrSpace;

    private:
        uint32_t PID;
        PCB();  // private PCB constctr for kernel

    public:
        ProcessState state;

        PCB(const char* binaryFile, ministl::unique_ptr<PageTable> pageSystem);

        static PCB& kernelThread() {
            static PCB kpcb;
            return kpcb;
        }
        
    };


}

inline kernel::PCB* currentThread;
// Current thread. When handleTrap returns, run_process(currentThread->regCtx) gets called.

extern "C" void run_process(kernel::RegisterContext& context); 
// Asmglue function. It will set up the registers based on the context reference, which is enough to run a thread.
// Using this has no guarantee of return (in fact, it probably won't ever return) and will likely corrupt the stack if it did.
// This is fine in the cppmain function because we don't care about it's stack or running anything after running the init process.
// However, this should be reserved for asmglue.

#endif