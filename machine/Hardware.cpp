#include "Hardware.h"
#include "BinaryUtils.h"
#include "instructions/Instruction.h"
#include <iostream>
#include <iomanip>

#define TEXT_ENTRY 0x004002c0
#define DATA_ENTRY 0x10008000

// TODO: add some debugging ability
// -------------------------------------------------------------
// Hardware Emulation
// Instruction factory can be in found in InstructionFactory.cpp
// -------------------------------------------------------------
Hardware::Machine::Machine() {
    for (int i = 0; i < 32; ++i) {
        registers.registerFile[i] = 0;
        registers.fpRegisterFile[i] = 0.0f;
    }

    registers.programCounter = TEXT_ENTRY;
    registers.registerFile[Binary::SP] = 0x7fffffff;
    registers.registerFile[Binary::FP] = 0x7fffffff;
    registers.registerFile[Binary::GP] = DATA_ENTRY; 
    killed = false;
    registers.FPcond = false;
}

const Word& Hardware::Machine::readProgramCounter() const {
    return registers.programCounter;
}

const int& Hardware::Machine::readRegister(const Byte& reg) const {
    return registers.registerFile[reg];
}

const Hardware::Memory& Hardware::Machine::readMemory() const {
    return registers.RAM;
}

const float& Hardware::Machine::readFPRegister(const Byte& reg) const {
    return registers.fpRegisterFile[reg];
}

void Hardware::Machine::loadData(const std::vector<Byte>& bytes) {

    Word at = DATA_ENTRY;
    for (const auto& byte : bytes) {
        registers.RAM.setByte(at, byte);
        ++at;
    }
    registers.RAM.memoryBounds.staticBound = at;
}

void Hardware::Machine::loadInstructions(const std::vector<Word>& instructions) {
    // for right now, just load according to mips for no patricular reason
    // will figure out exact specifications later

    Word at = TEXT_ENTRY;
    for (const auto& instr : instructions) {
        registers.RAM.setWord(at, instr);
        at += 4;
    }
    
    Memory::boundRegisters& bounds = registers.RAM.memoryBounds;
    bounds.textBound = at;
    bounds.stackBound = 0x7fffe000;
    bounds.dynamicBound = 0x70000000;
}

void Hardware::Machine::runInstruction() {
    if (registers.programCounter >= registers.RAM.memoryBounds.textBound) {
        killed = true;
        return;
    }


    auto it = instructionCache.find(registers.programCounter);
    if (it != instructionCache.end()) {
        it->second->run();
        registers.programCounter += 4;
        return;
    }

    (
        instructionCache[registers.programCounter] = instructionFactory( registers.RAM.getWord(registers.programCounter), registers, killed )
    )->run(); // cool syntax

    registers.programCounter += 4;
}

void Hardware::Machine::run(instrDebugHook hook) {
    while (!killed) {
        runInstruction();
        if (hook) hook(*this);
    }
}