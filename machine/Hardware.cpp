#include "Hardware.h"
#include "BinaryUtils.h"
#include "instructions/Instruction.h"

#define DATA_ENTRY 0x10008000
#define TEXT_START 0x00400024
#define KERNEL_TEXT_START 0x80000000
#define KERNEL_DATA_ENTRY 0x80001000

// -------------------------------------------------------------
// Hardware Emulation
// -------------------------------------------------------------

Hardware::Machine::Machine() : cpu(*this), killed(false) {

    cpu.accessRegister(Binary::SP).ui = 0x7ffffffc;
    cpu.accessRegister(Binary::GP).ui = DATA_ENTRY; 
    coprocessors[0] = std::make_unique<SystemControlUnit>(*this);
    coprocessors[1] = std::make_unique<FloatingPointUnit>(*this);
    coprocessors[2] = nullptr;
}

void Hardware::Machine::raiseTrap(const Byte& exceptionCode) {
    using namespace Binary;

    SystemControlUnit* sys_ctrl = dynamic_cast<SystemControlUnit*>(coprocessors[0].get());  // if this errors, we can just get rid of it since all that happens is reg interaction

    if (!(sys_ctrl->accessRegister(STATUS).ui & 0b10)) {
        sys_ctrl->setEPC( cpu.readProgramCounter() );
        sys_ctrl->setEXL(true);
    }

    sys_ctrl->setCause(exceptionCode);

    cpu.accessProgramCounter() = (exceptionCode == 24 ? bootEntry : trapEntry);

    // store trap frame
    auto& sp = cpu.accessRegister(SP).ui;
    Word old_sp = sp;
    Word end = 34 * 4;  // all registers - $0 + EPC + STATUS + CAUSE
    sp -= end;
    for (Byte i = 1; i < 32; ++i) RAM.setWord(sp + ((i - 1) << 2), cpu.accessRegister(i).ui );
    RAM.setWord(sp + SP * 4, old_sp);
    RAM.setWord(sp + 31 * 4, coprocessors[0]->readRegister(EPC).ui );
    RAM.setWord(sp + 32 * 4, coprocessors[0]->readRegister(STATUS).ui );
    RAM.setWord(sp + 33 * 4, coprocessors[0]->readRegister(CAUSE).ui );

    cpu.accessRegister(K0).ui = sp; // load in k0 so kernel can access it 
}

void Hardware::Machine::loadKernel(const std::vector<Word>& words, const std::vector<Byte>& bytes, const Word& entry, const Word& trapEntry) {
    this->trapEntry = trapEntry;
    bootEntry = entry;
    Word at = KERNEL_TEXT_START;
    for (const auto& instr : words) {
        RAM.setWord(at, instr);
        at += 4;
    }

    at = KERNEL_DATA_ENTRY;
    for (const auto& byte : bytes) {
        RAM.setByte(at, byte);
        ++at;
    }

}


void Hardware::Machine::loadProgram(const std::vector<Word>& instructions, const std::vector<Byte>& bytes, const Word& entry) {
    // for right now, just load according to mips for no patricular reason
    // will figure out exact specifications later
    Word at = TEXT_START;
    for (const auto& instr : instructions) {
        RAM.setWord(at, instr);
        at += 4;
    }

    at = DATA_ENTRY;
    for (const auto& byte : bytes) {
        RAM.setByte(at, byte);
        ++at;
    }

    cpu.accessProgramCounter() = entry;
    raiseTrap(24);  // boot

    // set memory bounds
}

void Hardware::Machine::step() {
    cpu.cycle();
}

void Hardware::Machine::run(instrDebugHook hook) {
    if (hook) hook(*this);
    while (!killed) {
        step();
        if (hook) hook(*this);
    }
}