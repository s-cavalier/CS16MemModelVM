#include "Hardware.h"
#include "BinaryUtils.h"
#include "instructions/Instruction.h"
#include <iostream>
#include <iomanip>

#define DATA_ENTRY 0x10008000
#define TEXT_START 0x00400024
#define KERNEL_TEXT_START 0x80000000
#define KERNEL_DATA_ENTRY 0x80005000
#define KERNEL_STACK_OFFSET 8188
#define KERNEL_GLOBAL_PTR_DEFAULT 0x80007500

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

    // store trap frame on kernel stack
    auto& ksp = coprocessors[0]->accessRegister(K_SP).ui;
    Word end = 34 * 4;  // all registers - $0 + EPC + STATUS + CAUSE
    ksp -= end;

    for (Byte i = 1; i < 32; ++i) RAM.setWord(ksp + ((i - 1) << 2), cpu.accessRegister(i).ui);

    RAM.setWord(ksp + SP * 4, cpu.accessRegister(SP).ui);  // save user $sp
    RAM.setWord(ksp + 31 * 4, coprocessors[0]->readRegister(EPC).ui);
    RAM.setWord(ksp + 32 * 4, coprocessors[0]->readRegister(STATUS).ui);
    RAM.setWord(ksp + 33 * 4, coprocessors[0]->readRegister(CAUSE).ui);

    cpu.accessRegister(K0).ui = ksp;  // store trap frame address in $k0
    coprocessors[0]->accessRegister(K_TF).ui = ksp;  // store in K_TF

    cpu.accessRegister(SP).ui = coprocessors[0]->accessRegister(K_SP).ui;  // switch to kernel stack
    cpu.accessRegister(GP).ui = KERNEL_GLOBAL_PTR_DEFAULT;

}

void Hardware::Machine::loadKernel(const ExternalInfo::KernelBootInformation& kernelInfo) {
    trapEntry = kernelInfo.trapEntry;
    bootEntry = kernelInfo.bootEntry;
    Word at = KERNEL_TEXT_START;
    for (const auto& instr : kernelInfo.text) {
        RAM.setWord(at, instr);
        at += 4;
    }

    at = KERNEL_DATA_ENTRY;
    for (const auto& byte : kernelInfo.data) {
        RAM.setByte(at, byte);
        ++at;
    }

    coprocessors[0]->accessRegister(Binary::K_SP).ui = kernelInfo.kernelStackPointerAddr + KERNEL_STACK_OFFSET;

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

    cpu.accessProgramCounter() = entry - 4;
    raiseTrap(24);  // boot

    // set memory bounds
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