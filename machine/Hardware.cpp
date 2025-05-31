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
Hardware::Machine::Machine() : cpu(*this), killed(false) {

    cpu.accessProgramCounter() = TEXT_ENTRY;
    cpu.accessRegister(Binary::SP).ui = 0x7fffffff;
    cpu.accessRegister(Binary::FP).ui = 0x7fffffff;
    cpu.accessRegister(Binary::GP).ui = DATA_ENTRY; 
    coprocessors[0] = std::make_unique<SystemControlUnit>(*this);
    coprocessors[1] = std::make_unique<FloatingPointUnit>(*this);
}

void Hardware::Machine::raiseTrap(const Byte& exceptionCode) {
    SystemControlUnit* sys_ctrl = dynamic_cast<SystemControlUnit*>(coprocessors[0].get());  // if this errors, we can just get rid of it since all that happens is reg interaction

    if (sys_ctrl->readEXL()) {
        sys_ctrl->setEPC( cpu.readProgramCounter() );
        sys_ctrl->setEXL(true);
    }

    sys_ctrl->setCause(exceptionCode);

}

void Hardware::Machine::loadData(const std::vector<Byte>& bytes) {

    Word at = DATA_ENTRY;
    for (const auto& byte : bytes) {
        RAM.setByte(at, byte);
        ++at;
    }
    RAM.memoryBounds.staticBound = at;
}

void Hardware::Machine::loadInstructions(const std::vector<Word>& instructions) {
    // for right now, just load according to mips for no patricular reason
    // will figure out exact specifications later

    Word at = TEXT_ENTRY;
    for (const auto& instr : instructions) {
        RAM.setWord(at, instr);
        at += 4;
    }
    
    Memory::boundRegisters& bounds = RAM.memoryBounds;
    bounds.textBound = at;
    bounds.stackBound = 0x7fffe000;
    bounds.dynamicBound = 0x70000000;
}

void Hardware::Machine::step() {
    cpu.cycle();
}

void Hardware::Machine::run(instrDebugHook hook) {
    while (!killed) {
        step();
        if (hook) hook(*this);
    }
}