#include "ASMInterface.h"
#include "Process.h"

enum ExceptionType : unsigned char {
        INTERRUPT = 0,   // Interrupt (hardware)
        TLB_MOD = 1,
        TLB_L = 2,
        TLB_S = 3,
        ADDRL = 4,   // Address error on load or fetch
        ADDRS = 5,   // Address error on store
        SYSCALL = 8,   // SYSCALL instruction
        BREAK = 9,   // BREAK instruction
        RI = 10,  // Reserved instruction
        CP_UNUSABLE = 11,  // Coprocessor unusable
        OVERFLOW = 12,  // Arithmetic overflow
        TRAP = 13,  // TRAP instruction (e.g., TEQ + trap conditions)
        FP_EXC = 15,  // Floating-point exception
        WATCH = 23,  // Watchpoint reference
        // Other values are reserved or implementation-specific
    };

    enum SyscallType : unsigned short {
        PRINT_INTEGER = 1,
        PRINT_STRING = 4,
        READ_INTEGER = 5,
        EXIT = 10
    };

// Called from handle_trap in asmglue.asm. Calls run_process(currentThread->ctx) on exit.
extern "C" void handleTrap() {
    assert(currentThread);
    kernel::RegisterContext* trapCtx = (kernel::RegisterContext*)kernel::getK0Register();
    size_t res = kernel::getBadVAddr();
    // The register context of the trapping thread is saved to k0. We need to use this context on the stack in case of >1 nested exceptions.

    kernel::PCB* oldThread = currentThread;                                     // save oldThread
    currentThread = &kernel::PCB::kernelThread();                                     // set currentThread to kernel PCB

    ExceptionType cause = ExceptionType(trapCtx->cause >> 2); // need to consider interrupt mask later

    switch (cause) {
        case TLB_L: { 
            oldThread->addrSpace.translate(res).writeRandom();
            trapCtx->epc -= 4;
            
            break;
        }
        case TLB_S: { 
            oldThread->addrSpace.translate(res).writeRandom();
            trapCtx->epc -= 4; 
            
            break;
        } 
        
        case SYSCALL: {
            switch ( trapCtx->accessRegister(kernel::V0) ) { 
                case PRINT_INTEGER:
                    PrintInteger( trapCtx->accessRegister(kernel::A0) );
                    break;  //shoudn't hit here
                case PRINT_STRING:
                    PrintString( trapCtx->accessRegister(kernel::A0) );
                    break;
                case READ_INTEGER:
                    trapCtx->accessRegister(kernel::V0) = ReadInteger.res;
                    break;
    
                case EXIT: {
                    Halt;
                    break;
                }
                default:
                    PrintString("[KERNEL] Unrecognized syscall code. Returning without doing anything.\n");
                    break;
            }
            
            break;
        }
        case RI:
            PrintString("[KERNEL] Attempted privilieged instruction outside of kernel. Killing process...\n");
            PrintInteger( trapCtx->epc );
            Halt;
            break;
        case OVERFLOW:
            PrintString("[KERNEL] Arithmetic overflow exception. Killing process...\n");
            Halt;
            break;
        default:
            PrintString("[KERNEL] Recieved unrecognized exception code ");
            PrintInteger(cause);
            PrintString("\n");
            break;
    }

    currentThread = oldThread;
    currentThread->regCtx = *trapCtx; // Save the registers to the current thread so asmglue can restore it later
}