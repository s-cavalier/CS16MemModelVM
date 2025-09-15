#include "Hardware.h"
#include "BinaryUtils.h"
#include "instructions/Instruction.h"

#ifdef DEBUG
    #include <iostream>
    #include <iomanip>
    #define debug(x) x
#else
    #define debug(x)
#endif

#define DATA_ENTRY 0x10008000
#define TEXT_START 0x00400024

// -------------------------------------------------------------
// Hardware Emulation
// -------------------------------------------------------------

Hardware::Machine::Machine() : cpu(*this), killed(false) {}

void Hardware::Machine::loadKernel(const ExternalInfo::KernelBootInformation& kernelInfo, const std::vector<std::string>& kernelArgs) {
    trapEntry = kernelInfo.trapEntry;
    Word at = kernelInfo.textStart;

    for (const auto& instr : kernelInfo.text) {
        memory.setWord(at, instr, cpu.tlb);
        at += 4;
    }

    at = kernelInfo.dataStart;
    for (const auto& byte : kernelInfo.data) {
        memory.setByte(at, byte, cpu.tlb);
        ++at;
    }

    memory.setWord(kernelInfo.argc, kernelArgs.size(), cpu.tlb);

    for (Word i = 0; i < kernelArgs.size(); ++i) {
        Word indirectPtr = kernelInfo.argv + 64 * i;    // argv[i] = *(argv + i)
        for (Word j = 0; j < kernelArgs[i].size(); ++j) memory.setByte(indirectPtr + j, kernelArgs[i][j], cpu.tlb ); // argv[i][j]
    }

    cpu.programCounter = kernelInfo.bootEntry;
    cpu.scu.registerFile[Binary::STATUS].ui = 0b10;  // enable exl at boot <- has to be done by hardware

}

// Runs a single cycle. Since the interrupt system is clock-based and not instr count based, interrupts are disabled
// Use a Hardware::SendInterrupt object if you want to manually call an interrupt
void Hardware::Machine::step() {
    cpu.cycle();
}


// Runs with an Interrupt Device.
void Hardware::Machine::run(instrDebugHook hook, std::chrono::milliseconds IDduration ) {
    assert(cpu.interDev);
    cpu.interDev->start(IDduration);
    while (!killed) {
        step();
        if (hook) hook(*this);
    }
    cpu.interDev->stop();
}