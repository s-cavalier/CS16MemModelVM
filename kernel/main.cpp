#include "HeapManager.h"
#include "ASMInterface.h"
// --- These should be the first includes, especially HeapManager so everything after is correctly linked with the new operator

// -- Stack Init --

#define K_STACK_SIZE 16384

__attribute__((aligned(K_STACK_SIZE)))
char kernel_stack[K_STACK_SIZE];



// -- Nice Utility --
unsigned int newline = (unsigned int)("\n");


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


int main() {
    // just eret assuming that EPC already has the right PC loaded
    call_global_constructors();
    PrintString("Kernel booted!\n");

    __asm__ volatile ("eret\n" : : :);
    return 0;
}

extern "C" void handleTrap() {

    kernel::TrapFrame* trapFrame = kernel::loadTrapFrame();

    unsigned int cause = trapFrame->cause >> 2; // need to consider interrupt mask later

    switch (cause) {
        case 8: {
            switch ( trapFrame->v0 ) { 
                case 1:
                    PrintInteger( trapFrame->a0 );
                    break;  //shoudn't hit here
                case 4:
                    PrintString( trapFrame->a0 );
                    break;
                case 5:
                    trapFrame->v0 = ReadInteger.res;
                    break;
                case 10: {
                    Halt;
                    break;
                }
                default:
                    PrintString("[KERNEL] Unrecognized syscall code. Returning without doing anything.\n");
                    break;
            }
            return;
        }
        case 10:
            PrintString("[KERNEL] Attempted privilieged instruction outside of kernel. Killing process...\n");
            PrintInteger( (unsigned int)(trapFrame->epc) );
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

    Halt;
}
