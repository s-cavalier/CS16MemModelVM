#include "KInstructions.h"
#include "../BinaryUtils.h"
#include <iostream>

#define EXL_CHECK if (!checkEXL()) { raiseTrap(10); return; }

KInstruction::KInstruction(K_INSTR_ARGS) : 
    raiseTrap(raiseTrap), statusRegister( statusRegister ) {}

MoveToCoprocessor0::MoveToCoprocessor0(K_INSTR_ARGS, const int& rt, int& rd) : 
    KInstruction(raiseTrap, statusRegister), rt(rt), rd(rd) {}

void MoveToCoprocessor0::run() { EXL_CHECK rd = rt; }

MoveFromCoprocessor0::MoveFromCoprocessor0(K_INSTR_ARGS, int& rt, const int& rd) : 
    KInstruction(raiseTrap, statusRegister), rt(rt), rd(rd) {}

void MoveFromCoprocessor0::run() { EXL_CHECK rt = rd; }

ExceptionReturn::ExceptionReturn(Hardware::Machine& machine) : KInstruction(machine.accessTrapHandler(), machine.accessCoprocessor(0)->accessRegister(Binary::STATUS).ui ), machine(machine) {}

void ExceptionReturn::run() { 
    EXL_CHECK 
    machine.accessCPU().accessProgramCounter() = machine.readCoprocessor(0)->readRegister(Binary::EPC).ui;
    // don't need to flip exl bit, just restore the status with trap frame

    auto& spu = machine.accessCoprocessor(0);

    // restore trap frame based on value in $k0
    Word trapFramePointer = machine.readCPU().readRegister(Binary::K0).ui;
    for (Byte i = 1; i < 32; ++i) machine.accessCPU().accessRegister(i).ui = machine.readMemory().getWord(trapFramePointer + ((i - 1) << 2));
    spu->accessRegister(Binary::EPC).ui = machine.readMemory().getWord(trapFramePointer + 31 * 4);
    spu->accessRegister(Binary::STATUS).ui = machine.readMemory().getWord(trapFramePointer + 32 * 4);
    spu->accessRegister(Binary::STATUS).ui &= ~Word(0b10);  //flip exl
    spu->accessRegister(Binary::CAUSE).ui = machine.readMemory().getWord(trapFramePointer + 33 * 4);
}



// custom
Halt::Halt(K_INSTR_ARGS, bool& kill) : KInstruction(raiseTrap, statusRegister), kill(kill) {}
void Halt::run() { EXL_CHECK kill = true; }

PrintInteger::PrintInteger(K_INSTR_ARGS, const int& a0) : KInstruction(raiseTrap, statusRegister), a0(a0) {}
void PrintInteger::run() { EXL_CHECK std::cout << a0; }

ReadInteger::ReadInteger(K_INSTR_ARGS, int& v0) : KInstruction(raiseTrap, statusRegister), v0( v0 ) {}
void ReadInteger::run() { EXL_CHECK std::cin >> v0; }

PrintString::PrintString(K_INSTR_ARGS, Hardware::Memory& mem, const Word& a0) : KInstruction(raiseTrap, statusRegister), mem( mem ), a0( a0 ) {}
void PrintString::run() { EXL_CHECK for (Word i = a0; mem.getByte(i) != '\0'; ++i) std::cout << mem.getByte(i); }