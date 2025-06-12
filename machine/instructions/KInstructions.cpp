#include "KInstructions.h"
#include "../BinaryUtils.h"
#include <iostream>

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
    auto& spu = machine.accessCoprocessor(0);

    machine.accessCPU().accessProgramCounter() = spu->readRegister(Binary::EPC).ui;
    // don't need to flip exl bit, just restore the status with trap frame

    // restore trap frame based on value in $k0
    Word trapFramePointer = spu->accessRegister(Binary::K_TF).ui;

    for (Byte i = 1; i < 32; ++i)
        machine.accessCPU().accessRegister(i).ui = machine.readMemory().getWord(trapFramePointer + ((i - 1) << 2));

    spu->accessRegister(Binary::EPC).ui = machine.readMemory().getWord(trapFramePointer + 31 * 4);
    spu->accessRegister(Binary::STATUS).ui = machine.readMemory().getWord(trapFramePointer + 32 * 4);
    spu->accessRegister(Binary::STATUS).ui &= ~Word(0b10);  // clear EXL
    spu->accessRegister(Binary::CAUSE).ui = machine.readMemory().getWord(trapFramePointer + 33 * 4);

    // restore user $sp (which was saved in the trap frame)
    machine.accessCPU().accessRegister(Binary::SP).ui = machine.readMemory().getWord(trapFramePointer + Binary::SP * 4);

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

    switch (req) {
        case 1: // halt
            machine.killed = true;
            break;

        case 2: // print string
            for (Word i = arg0; machine.readMemory().getByte(i) != '\0'; ++i) std::cout << machine.readMemory().getByte(i);
            break;

        case 3: // print integer
            std::cout << arg0;
            break;
        
        case 4: // read integer
            std::cin >> res;
            break;
        default:
            break;
    }

    // load back into res, err
    
    machine.accessMemory().setWord( resAddr, res     );
    machine.accessMemory().setWord( resAddr + 4, err );

}
