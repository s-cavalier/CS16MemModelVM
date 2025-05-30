#include "Hardware.h"
#include "BinaryUtils.h"
#include "Instruction.h"
#include <iostream>
#include <iomanip>

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

    registers.programCounter = 0x00400024;
    registers.registerFile[Binary::SP] = 0x7fffffff;
    registers.registerFile[Binary::FP] = 0x7fffffff;
    registers.registerFile[Binary::GP] = 0x10008000; 
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
    std::cout << "Loaded Data Section: " << bytes.size() << " bytes loaded" << std::endl;

    Word at = 0x10008000;
    for (const auto& byte : bytes) {
        registers.RAM.setByte(at, byte);
        ++at;
    }
    registers.RAM.memoryBounds.staticBound = at;
}

void Hardware::Machine::loadInstructions(const std::vector<Word>& instructions) {
    // for right now, just load according to mips for no patricular reason
    // will figure out exact specifications later

    std::cout << "Loaded Text Section: " << instructions.size() * 4 << " bytes loaded" << std::endl;

    Word at = 0x00400024;
    for (const auto& instr : instructions) {
        registers.RAM.setWord(at, instr);
        at += 4;
    }
    
    Memory::boundRegisters& bounds = registers.RAM.memoryBounds;
    bounds.textBound = at;
    std::cout << "Placed PC bound at: " << std::hex << at << std::endl;
    bounds.stackBound = 0x7fffe000;
    bounds.dynamicBound = 0x70000000;
}

void Hardware::Machine::runInstruction() {
    std::cout << "At program counter " << std::hex << registers.programCounter << std::endl;

    if (registers.programCounter >= registers.RAM.memoryBounds.textBound) {
        std::cout << "Reading past text memory. Killing process..." << std::endl;
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

void Hardware::Machine::run() {
    while (!killed) runInstruction();
}