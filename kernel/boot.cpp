#include "HeapManager.h"
#include "Process.h"
// --- These should be the first includes, especially HeapManager so everything after is correctly linked with the new operator

#include "kstl/File.h"
#include "kstl/String.h"

// -- Stack Init --

#define K_STACK_SIZE 8192

__attribute__((aligned(K_STACK_SIZE)))
char kernel_stack[K_STACK_SIZE];
extern "C" char* const top_kstack = kernel_stack + K_STACK_SIZE; // don't want the pointer to be mutable, but the values within can be mutable

// -- Nice Utility --
unsigned int newline = (unsigned int)("\n");

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

kernel::PCB* currentThread;
// Current thread. When handleTrap returns, run_process(currentThread->regCtx) gets called.

extern "C" void run_process(kernel::RegisterContext& context); 
// Asmglue function. It will set up the registers based on the context reference, which is enough to run a thread.
// Using this has no guarantee of return (in fact, it probably won't ever return) and will likely corrupt the stack if it did.
// This is fine in the cppmain function because we don't care about it's stack or running anything after running the init process.
// However, this should be reserved for asmglue.

unsigned char exceptionDepth = 1; // asmglue.asm will handle this

extern "C" __attribute__((noreturn)) void cppmain() {
    // just eret assuming that EPC already has the right PC loaded
    call_global_constructors();
    PrintString("Kernel booted!\n");

    bool fromSpim = (argc > 1 && ministl::streq(argv[1], "-spim"));

    currentThread = new kernel::PCB(argv[0], fromSpim);

    exceptionDepth -= 1;
    run_process(currentThread->regCtx);

    //  return here kills the system, so hopefully it doesn't run
}


// Called from handle_trap in asmglue.asm. Calls run_process(currentThread->ctx) on exit.
extern "C" void handleTrap() {
    assert(currentThread);
    kernel::RegisterContext* trapCtx = (kernel::RegisterContext*)kernel::getK0Register();
    // The register context of the trapping thread is saved to k0. We need to use this context on the stack in case of >1 nested exceptions.

    kernel::PCB* oldThread = currentThread;                                     // save oldThread
    currentThread = &kernel::PCB::kernel();                                     // set currentThread to kernel PCB

    unsigned int cause = trapCtx->cause >> 2; // need to consider interrupt mask later

    switch (cause) {
        case 8: {
            switch ( trapCtx->accessRegister(kernel::V0) ) { 
                case 1:
                    PrintInteger( trapCtx->accessRegister(kernel::A0) );
                    break;  //shoudn't hit here
                case 4:
                    PrintString( trapCtx->accessRegister(kernel::A0) );
                    break;
                case 5:
                    trapCtx->accessRegister(kernel::V0) = ReadInteger.res;
                    break;
    
                case 10: {
                    Halt;
                    break;
                }
                default:
                    PrintString("[KERNEL] Unrecognized syscall code. Returning without doing anything.\n");
                    break;
            }
            
            break;
        }
        case 10:
            PrintString("[KERNEL] Attempted privilieged instruction outside of kernel. Killing process...\n");
            PrintInteger( trapCtx->epc );
            Halt;
            break;
        case 12:
            PrintString("[KERNEL] Arithmetic overflow exception. Killing process...\n");
            Halt;
            break;
        default:
            PrintString("[KERNEL] Recieved unrecognized exception code ");
            PrintInteger(cause);
            PrintString(newline);
    }

    currentThread = oldThread;
    currentThread->regCtx = *trapCtx; // Save the registers to the current thread so asmglue can restore it later
}
