#include "KInstructions.h"
#include "../BinaryUtils.h"
#include <iostream>

KInstruction::KInstruction(Hardware::Machine& machine) : 
    raiseTrap(machine), statusRegister( machine.accessCoprocessor(0)->accessRegister(Binary::STATUS).ui ) {}

MoveToCoprocessor0::MoveToCoprocessor0(Hardware::Machine& machine, const int& rt, int& rd) : 
    KInstruction(machine), rt(rt), rd(rd) {}

void MoveToCoprocessor0::run() { checkEXL(); rd = rt; }

MoveFromCoprocessor0::MoveFromCoprocessor0(Hardware::Machine& machine, int& rt, const int& rd) : 
    KInstruction(machine), rt(rt), rd(rd) {}

void MoveFromCoprocessor0::run() { checkEXL(); rt = rd; }

ExceptionReturn::ExceptionReturn(Hardware::Machine& machine) : KInstruction(machine), machine(machine) {}

void ExceptionReturn::run() { 
    checkEXL(); 
    machine.accessCPU().accessProgramCounter() = machine.readCoprocessor(0)->readRegister(Binary::EPC).ui;
    // don't need to flip exl bit, just restore the status with trap frame

    // restore trap frame based on value in $k0
    Word trapFramePointer = machine.readCPU().readRegister(Binary::K0).ui;
    for (Byte i = 1; i < 32; ++i) machine.accessCPU().accessRegister(i).ui = machine.readMemory().getWord(trapFramePointer + ((i - 1) << 2));
    machine.accessCoprocessor(0)->accessRegister(Binary::EPC).ui = machine.readMemory().getWord(trapFramePointer + 31 * 4);
    machine.accessCoprocessor(0)->accessRegister(Binary::STATUS).ui = machine.readMemory().getWord(trapFramePointer + 32 * 4);
    machine.accessCoprocessor(0)->accessRegister(Binary::CAUSE).ui = machine.readMemory().getWord(trapFramePointer + 33 * 4);
}



// custom
Halt::Halt(Hardware::Machine& machine) : KInstruction(machine), kill(machine.killed) {}
void Halt::run() { checkEXL(); kill = true; }

PrintInteger::PrintInteger(Hardware::Machine& machine) : KInstruction(machine), a0(machine.readCPU().readRegister(Binary::A0).i) {}
void PrintInteger::run() { checkEXL(); std::cout << a0; }

ReadInteger::ReadInteger(Hardware::Machine& machine) : KInstruction(machine), v0( machine.accessCPU().accessRegister(Binary::V0).i ) {}
void ReadInteger::run() { checkEXL(); std::cin >> v0; }

PrintString::PrintString(Hardware::Machine& machine) : KInstruction(machine), mem( machine.readMemory() ), a0( machine.readCPU().readRegister(Binary::A0).ui ) {}
void PrintString::run() { checkEXL(); for (Word i = a0; mem.getByte(i) != '\0'; ++i) std::cout << mem.getByte(i); }