#include "ASMInterface.h"
#include "SharedResources/Manager.h"
#include "KernelObjects/Scheduler.h"

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
        READ_STRING = 6,
        EXIT = 10,
        EXEC = 11,
        BRK = 45,
        FORK = 57
    };


// Called from handle_trap in asmglue.asm. Calls run_process(currentThread->ctx) on exit.
extern "C" void handleTrap() {
    assert(currentThread);
    kernel::RegisterContext* trapCtx = (kernel::RegisterContext*)kernel::getK0Register();
    size_t badVAddr = kernel::getBadVAddr();
    // The register context of the trapping thread is saved to k0. We need to use this context on the stack in case of >1 nested exceptions.

    kernel::PCB::Guard oldThread = currentThread->borrow();                                     // save oldThread
    currentThread = &kernel::sharedResources.processes.kernelProcess;          // set currentThread to kernel PCB

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
                case PRINT_STRING:{ // For some reason, the VMTUNNEL instr struggles with loading user-space addresses, so for now we'll grab it from physical memory
                    // That's fine for now, since we'll have to switch to just open/read/write/close later on
                    size_t physAddr = oldThread->addrSpace.translate( trapCtx->accessRegister(kernel::A0) ).lo >> 6;
                    physAddr <<= 12;
                    physAddr |= ( trapCtx->accessRegister(kernel::A0) & 0xFFF );

                    PrintString( 0x80000000 + physAddr ); // Temporary solution ; when FileSystem is implemented this will be nicer
                    break;
                }
                case READ_INTEGER:
                    trapCtx->accessRegister(kernel::V0) = ReadInteger.res;
                    break;
    
                case READ_STRING: {
                    size_t bufsize = trapCtx->accessRegister( kernel::A1 );
                    if ( bufsize > 4096 ) {
                        PrintString("[KERNEL] Max read is 4096 bytes. Returning with no-op...\n");
                        trapCtx->accessRegister(kernel::V0) = 0;
                        break;
                    }

                    char buf[bufsize];
                    size_t bytesRead = ReadString(buf, bufsize).res;

                    trapCtx->accessRegister(kernel::V0) = bytesRead;
                    if (bytesRead == 0 || bytesRead == -1) break;

                    // Currently swap addrspace and thread so any exceptions (TLB misses) get forwarded properly
                    oldThread.setAsCurrentThread();
                    kernel::replaceASID( currentThread->addrSpace.getASID() );

                    char* loc = (char*)trapCtx->accessRegister(kernel::A0);
                    for (size_t i = 0; i < bytesRead; ++i) {
                        loc[i] = buf[i];
                    }
                    
                    currentThread = &kernel::sharedResources.processes.kernelProcess;
                    break;
                }

                case EXIT: {
                    oldThread->markForDeath();
                    oldThread.reset();
                    break;
                }
                case EXEC: {
                    size_t physAddr = oldThread->addrSpace.translate( trapCtx->accessRegister(kernel::A0) ).lo >> 6;
                    physAddr <<= 12;
                    physAddr |= ( trapCtx->accessRegister(kernel::A0) & 0xFFF );
                    oldThread->exec( (const char*)(0x80000000 + physAddr), trapCtx );
                    break;
                }
                case BRK: {
                    bool validBrkRequest = oldThread->addrSpace.updateBrk( trapCtx->accessRegister(kernel::A0) );
                    if (validBrkRequest) break;
                    PrintString("[KERNEL] Bad BRK request. Killing process...\n");
                    oldThread->markForDeath();
                    oldThread.reset();
                    break;
                }
                case FORK: {
                    unsigned int newPID = kernel::sharedResources.processes.forkProcess( oldThread->getPID(), trapCtx );
                    kernel::PCB::Guard newProc = kernel::sharedResources.processes[newPID];
                    
                    newProc->regCtx.accessRegister( kernel::V0 ) = 0;
                    trapCtx->accessRegister( kernel::V0 ) = newPID;

                    kernel::MultiLevelQueue::scheduler.enqueue( kernel::sharedResources.processes[newPID] );
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

    if ( oldThread.valid() ) {
        if (exceptionDepth > 1 || cause == TLB_L || cause == TLB_S ) {
            oldThread.setAsCurrentThread();
            currentThread->regCtx = *trapCtx;
            return;
        }

        oldThread->regCtx = *trapCtx;

        kernel::MultiLevelQueue::scheduler.enqueue( oldThread->borrow() );
    }

    if ( kernel::MultiLevelQueue::scheduler.empty() ) {
        currentThread = &kernel::sharedResources.processes.kernelProcess;
        PrintString("No more processes exist! Killing system...\n");
        Halt;
    }

    kernel::clearICache();

    kernel::PCB::Guard incomingProc = kernel::MultiLevelQueue::scheduler.dequeue();

    kernel::replaceASID( incomingProc->addrSpace.getASID() );

    incomingProc.setAsCurrentThread();

}