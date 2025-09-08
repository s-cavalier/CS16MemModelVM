#ifndef __PROCESS_H__
#define __PROCESS_H__
#include "ASMInterface.h"
#include "kstl/Vector.h"
#include "VirtualMemory.h"
#include "kstl/Optional.h"

namespace kernel {

    enum ProcessState : unsigned char {
        READY,
        RUNNING,
        BLOCKED,
        ZOMBIE
    };

    struct PCB {
        RegisterContext regCtx;
        AddressSpace addrSpace;
        ProcessState state;
        unsigned char priority;

        uint32_t getPID() const { return PID; }

    private:
        uint32_t PID;
        PCB();
        PCB(uint32_t pid, ProcessState state, ministl::unique_ptr<PageTable> pageSystem);

        // Special boot-initalizer for the kernel PCB
        // Mainly needed to set currentThread = this when booting before initalizing ProcessManager's processes and freePids vectors (because they need dynamic memory, which needs to KPT to be setup)
        PCB(kernelInit_t, KernelPageTable& kpt);

        // PCB creation should go strictly through the ProcessManager
        friend class ProcessManager;
    
    };



    // ---------------------
    // Global ProcessManager
    // ---------------------
    // This should keep track and be the authoritative source of truth and owner of all processes/pids and their respective fields.
    // To avoid static init order fiasco with the dependencies on the needed statics ( kernelProcess needs KernelPageTable after AddressSpace is initalized, since both need to be static to avoid any other declarations )
    // we have this to manage those. Other statics/singletons may exist but this avoids SIOF as well as manage processes.
    class ProcessManager {
        KernelPageTable kPageTable; // Needs to be defined before kernelProcess
        ProcessManager();

    public:
        static constexpr uint32_t KERNEL_PID = 0xFFFFFFFF;
        static ProcessManager instance;
        PCB kernelProcess;

        // Results should be checked for nullptr if the process doesn't exist
        PCB* operator[](size_t idx);
        const PCB* operator[](size_t idx) const;
        
        // Returns a pid to a created process
        // Returns -1 (basically KERNEL_PID) on failure (i.e., bad filename)
        uint32_t createProcess(const char* executableFile);

        uint32_t forkProcess(uint32_t pid);
        
        // Marks a process ZOMBIE
        bool killProcess();

    private:
        ministl::vector< ministl::unique_ptr<PCB> > processes;
        ministl::vector< uint32_t > freePids;
    };

    static constexpr uint32_t NOPCBEXISTS = 0xFFFFFFFF;

}

// Current thread. When handleTrap returns, run_process(currentThread->regCtx) gets called.
// Isolated and marked inline to avoid name-mangling.
inline kernel::PCB* currentThread;

// Asmglue function. It will set up the registers based on the context reference, which is enough to run a thread.
// Using this has no guarantee of return (in fact, it probably won't ever return in the convential sense) and will likely corrupt the stack if it did.
// Thus it should be reserved for pure asm files to avoid compiler stack prolouge/epilouge.
// It's just here to be known as the way of doing an eret.
extern "C" void run_process(kernel::RegisterContext& context); 




#endif