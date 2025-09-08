#include "ASMInterface.h"
#include "Process.h"
#include "Scheduler.h"

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
        EXIT = 10,
        BRK = 45,
    };


// Called from handle_trap in asmglue.asm. Calls run_process(currentThread->ctx) on exit.
extern "C" void handleTrap() {
    assert(currentThread);
    kernel::RegisterContext* trapCtx = (kernel::RegisterContext*)kernel::getK0Register();
    size_t badVAddr = kernel::getBadVAddr();
    // The register context of the trapping thread is saved to k0. We need to use this context on the stack in case of >1 nested exceptions.

    kernel::PCB* oldThread = currentThread;                                     // save oldThread
    currentThread = &kernel::ProcessManager::instance.kernelProcess;          // set currentThread to kernel PCB

    ExceptionType cause = ExceptionType(trapCtx->cause >> 2); // need to consider interrupt mask later

    switch (cause) {
        case INTERRUPT:
            trapCtx->epc -= 4; // Due to some weird VM logic this needs to be shifted by 4 otherwise an instruction is skipped
            break;

        case TLB_L: {
            oldThread->addrSpace.translate(badVAddr).writeRandom();
            trapCtx->epc -= 4;
            
            break;
        }
        case TLB_S: {
            oldThread->addrSpace.translate(badVAddr).writeRandom();
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
                case BRK: {
                    bool validBrkRequest = oldThread->addrSpace.updateBrk( trapCtx->accessRegister(kernel::A0) );
                    if (validBrkRequest) break;
                    PrintString("[KERNEL] Bad BRK request. Killing process...\n");
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

    // Special case until we have ASID-based TLB entries and kernel shouldn't be scheduled
    if (oldThread->getPID() == kernel::ProcessManager::KERNEL_PID || cause == TLB_L || cause == TLB_S ) {
        currentThread = oldThread;
        currentThread->regCtx = *trapCtx;
        return;
    }

    oldThread->regCtx = *trapCtx;

    kernel::MultiLevelQueue::scheduler.enqueue(oldThread);
    unsigned int newPid = kernel::MultiLevelQueue::scheduler.dequeue();
    assert(newPid != kernel::NOPCBEXISTS);

    kernel::clearICache();
    for (size_t i = 0; i < 64; ++i) kernel::TLBEntry::invalidate(i);
    kernel::PCB* incomingProc = kernel::ProcessManager::instance[newPid];


    currentThread = incomingProc;

}