#include "HeapManager.h"
#include "Process.h"
// --- These should be the first includes, especially HeapManager so everything after is correctly linked with the new operator

#include "Scheduler.h"
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

unsigned char exceptionDepth = 1; // asmglue.asm will handle this, except in the cppmain boot

// Recall that ProcessManager sets currentThread = &kernel::ProcessManager::instace().kernelProcess during call_global_constructors

extern "C" void cppmain() {
    call_global_constructors(); // Should create ProcessManager
    PrintString("Kernel booted!\n");
    assert((argc > 0) && "Need a first process to run!");

    unsigned int firstPID = kernel::ProcessManager::instance.createProcess(argv[0]);
    unsigned int secondPID = kernel::ProcessManager::instance.forkProcess(firstPID);

    kernel::MultiLevelQueue::scheduler.enqueue( kernel::ProcessManager::instance[secondPID] );

    exceptionDepth -= 1;
    
    kernel::ProcessManager::instance[firstPID].setAsCurrentThread();
}   // Return here calls run_process(currentThread->regCtx)



