#include "TrapInits.h"

// should probably make the save/restore registers a core feature of the hardware traphandler

unsigned int newline = (unsigned int)("\n");

int main() {
    kernel::TrapFrame* trapFrame = kernel::loadTrapFrame();

    unsigned int cause = trapFrame->cause >> 2; // need to consider interrupt mask later
    
    switch (cause) {
        case 8: {
            switch ( trapFrame->v0 ) { 
                case 1:
                    kernel::printInteger( trapFrame->a0 );
                    break;  //shoudn't hit here
                case 4:
                    kernel::printString( trapFrame->a0 );
                    break;
                case 5:
                    trapFrame->v0 = kernel::readInteger();
                    break;
                case 10:
                    kernel::halt();
                    break;
                default:
                    kernel::printString("[KERNEL] Unrecognized syscall code. Returning without doing anything.\n");
                    break;
            }
            kernel::ereturn();
            break;  // should never hit here
        }
        case 10:
            kernel::printString("[KERNEL] Attempted privilieged instruction outside of kernel. Killing process...");
            kernel::halt();
            break;
        default:
            kernel::printString("Recieved unrecognized exception code ");
            kernel::printInteger(cause);
            kernel::printString(newline);
    }

    kernel::halt();
}