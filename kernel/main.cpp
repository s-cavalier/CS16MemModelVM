#include "ASMInterface.h"
#include "HeapManager.h"

#define K_STACK_SIZE 8192

__attribute__((aligned(K_STACK_SIZE)))
char kernel_stack[K_STACK_SIZE];

unsigned int newline = (unsigned int)("\n");

int main() {
    // just eret assuming that EPC already has the right PC loaded
    const char* startup = "Booted kernel!\n";
    PrintString(startup);

    int* x = new int[5]();

    delete[] x;
    PrintInteger((unsigned int)Heap::freeList.sbrk(0));
    PrintString(newline);

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
                case 10:
                    Halt;
                    break;
                default:
                    PrintString("[KERNEL] Unrecognized syscall code. Returning without doing anything.\n");
                    break;
            }
            return;
        }
        case 10:
            PrintString("[KERNEL] Attempted privilieged instruction outside of kernel. Killing process...\n");
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
