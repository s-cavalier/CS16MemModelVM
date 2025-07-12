#include "ASMInterface.h"
#include "Process.h"

// Called from handle_trap in asmglue.asm. Calls run_process(currentThread->ctx) on exit.
extern "C" void handleTrap() {
    assert(currentThread);
    kernel::RegisterContext* trapCtx = (kernel::RegisterContext*)kernel::getK0Register();
    // The register context of the trapping thread is saved to k0. We need to use this context on the stack in case of >1 nested exceptions.

    kernel::PCB* oldThread = currentThread;                                     // save oldThread
    currentThread = &kernel::PCB::kernelThread();                                     // set currentThread to kernel PCB

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
            PrintString("\n");
    }

    currentThread = oldThread;
    currentThread->regCtx = *trapCtx; // Save the registers to the current thread so asmglue can restore it later
}