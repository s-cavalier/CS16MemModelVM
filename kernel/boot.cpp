#include "SharedResources/Manager.h"
// --- These should be the first includes, especially HeapManager so everything after is correctly linked with the new operator

// -- Stack Init --

constexpr unsigned int K_STACK_SIZE = 16_kb;

__attribute__((aligned(K_STACK_SIZE)))
char kernel_stack[K_STACK_SIZE];
extern "C" char* const top_kstack = kernel_stack + K_STACK_SIZE; // don't want the pointer to be mutable, but the values within can be mutable

// -- Command Line Args --
unsigned int argc;
char argv[4][64]; // 4 args of length 64 (total 256 chars) not including prog name (this is the kernel not a user process, the name is always kernel.elf)

// -- Init Array --
extern "C" {
    typedef void (*Constructor)();
    extern Constructor __init_array_start[];
    extern Constructor __init_array_end[];
}

void call_global_constructors() {
    for (Constructor* func = __init_array_start; func < __init_array_end; ++func) {
        (*func)();
    }
}


// Recall that ProcessManager sets currentThread = &kernel::ProcessManager::instace().kernelProcess during call_global_constructors

extern "C" void cppmain() {
    call_global_constructors(); // Should create ProcessManager
    PrintString("Kernel booted!\n");


    unsigned int firstPID = kernel::sharedResources.processes.createProcess("programs/shell/shell.elf");

    exceptionDepth -= 1;

    kernel::PCB::Guard firstProc = kernel::sharedResources.processes[firstPID];
    
    kernel::replaceASID( firstProc->addrSpace.getASID() );


    firstProc.setAsCurrentThread();
}   // Return here calls run_process(currentThread->regCtx)



