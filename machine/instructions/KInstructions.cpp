#include "KInstructions.h"
#include "../BinaryUtils.h"
#include <iostream>
#include <string>

#define EXL_CHECK if (!checkEXL()) throw Hardware::Trap(Hardware::Trap::RI)

KInstruction::KInstruction(K_INSTR_ARGS) :  statusRegister( statusRegister ) {}

MoveToCoprocessor0::MoveToCoprocessor0(K_INSTR_ARGS, const int& rt, int& rd) : 
    KInstruction(statusRegister), rt(rt), rd(rd) {}

void MoveToCoprocessor0::run() { EXL_CHECK; rd = rt; }

MoveFromCoprocessor0::MoveFromCoprocessor0(K_INSTR_ARGS, int& rt, const int& rd) : KInstruction(statusRegister), rt(rt), rd(rd) {}

void MoveFromCoprocessor0::run() { EXL_CHECK; rt = rd; }

ExceptionReturn::ExceptionReturn(Hardware::Machine& machine) : KInstruction(machine.accessCoprocessor(0)->accessRegister(Binary::STATUS).ui ), machine(machine) {}

void ExceptionReturn::run() { 
    EXL_CHECK; 
    machine.accessCPU().accessProgramCounter() = machine.accessCoprocessor(0)->readRegister(Binary::EPC).ui;

}



// custom
VMTunnel::VMTunnel(K_INSTR_ARGS, Hardware::Machine& machine) : KInstruction(statusRegister), machine(machine) {}
void VMTunnel::run() {
    EXL_CHECK;
    
    auto& cpu = machine.accessCPU();

    Word reqAddr = cpu.readRegister(Binary::A0).ui;
    Word resAddr = cpu.readRegister(Binary::V0).ui;

    // this can be optimized pretty heavily once we switch to virtual memory
    Word req  = machine.readMemory().getWord( reqAddr );
    Word arg0 = machine.readMemory().getWord( reqAddr + 4 );
    Word arg1 = machine.readMemory().getWord( reqAddr + 8 );
    Word arg2 = machine.readMemory().getWord( reqAddr + 12 );

    Word res = 0;
    Word err = 0;


    // todo: do some optimizations with the string inputs
    switch (req) {
        case 1: // halt()
            machine.killed = true;

            break;

        case 2: // printString(const char*)
            for (Word i = arg0; machine.readMemory().getByte(i) != '\0'; ++i) std::cout << machine.readMemory().getByte(i);

            break;

        case 3: // printInteger(int)
            std::cout << arg0;

            break;
        
        case 4: // readInteger()
            std::cin >> res;

            break;

        case 5: { // fopen(const char* pathname, uint flags) -- need to consider error handling
            std::string filePath;
            for (Word i = arg0; machine.readMemory().getByte(i) != '\0'; ++i) filePath.push_back( machine.readMemory().getByte(i) ); 
            res = machine.accessFileSystem().open(filePath, arg1);

            break;
        }
        case 6: { // fread(int fd, char* buf, int nbytes)
            auto bytes = machine.accessFileSystem()[arg0]->read(arg2);
            for (Word i = 0; i < bytes.size(); ++i) machine.accessMemory().setByte(arg1 + i, bytes[i]);

            break;
        }
        case 7: // fwrite

        case 8: // fseek

        case 9: // fclose(int fd)
            machine.accessFileSystem().close(arg0);

            break;
        default:
            break;
    }

    // load back into res, err
    
    machine.accessMemory().setWord( resAddr, res     );
    machine.accessMemory().setWord( resAddr + 4, err );

}
