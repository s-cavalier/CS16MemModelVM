#include "HeapManager.h"
#include "Process.h"
// --- These should be the first includes, especially HeapManager so everything after is correctly linked with the new operator

#include "kstl/File.h"
#include "kstl/String.h"

#include "PageTables.h"
// -- Stack Init --

#define K_STACK_SIZE 8192

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

unsigned char exceptionDepth = 1; // asmglue.asm will handle this

struct Foo {
    Foo() { PrintString("hello from foo\n"); }
    void bar() { PrintString("bar\n"); }
    ~Foo() { PrintString("goodbye from foo\n"); }
};

extern "C" void cppmain() {
    // just eret assuming that EPC already has the right PC loaded
    call_global_constructors();
    currentThread = &kernel::PCB::kernelThread();
    PrintString("Kernel booted!\n");
    
    auto firstProc = new kernel::PCB(argv[0], ministl::make_unique<kernel::SegmentedPageTable>(16, 8) );

    currentThread = firstProc;

    exceptionDepth -= 1;
    
}   // Return here calls run_process(currentThread->regCtx)



