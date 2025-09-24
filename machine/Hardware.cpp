#include "Hardware.h"
#include "BinaryUtils.h"
#include "instructions/Instruction.h"
#include "KernelElf.h"
#include <iostream>
#include <unistd.h>

#ifdef DEBUG
    #include <iomanip>
    #define debug(x) x
#else
    #define debug(x)
#endif

// -------------------------------------------------------------
// Hardware Emulation
// -------------------------------------------------------------

size_t Hardware::Terminal::read(char* buf, size_t bytes) {
    return ::read(STDIN_FILENO, buf, bytes);
}
void Hardware::Terminal::write(const std::string& data) {
    std::cout << data << std::flush;
}

Hardware::Machine::Machine( std::unique_ptr<stdIODevice> stdiodev ) : cpu(*this), killed(false) {
    if (!stdiodev) stdio = std::make_unique<Terminal>();
    else stdio = std::move( stdiodev );
}

void Hardware::Machine::loadKernel(const ExternalInfo::KernelBootInformation& kernelInfo, const std::vector<std::string>& kernelArgs) {
    trapEntry = kernelInfo.trapEntry;
    Word at = kernelInfo.textStart;

    for (const auto& instr : kernelInfo.text) {
        memory.setWord(at, instr, cpu.tlb, 0);
        at += 4;
    }

    at = kernelInfo.dataStart;
    for (const auto& byte : kernelInfo.data) {
        memory.setByte(at, byte, cpu.tlb, 0);
        ++at;
    }

    memory.setWord(kernelInfo.argc, kernelArgs.size(), cpu.tlb, 0);

    for (Word i = 0; i < kernelArgs.size(); ++i) {
        Word indirectPtr = kernelInfo.argv + 64 * i;    // argv[i] = *(argv + i)
        for (Word j = 0; j < kernelArgs[i].size(); ++j) memory.setByte(indirectPtr + j, kernelArgs[i][j], cpu.tlb, 0 ); // argv[i][j]
    }

    cpu.programCounter = kernelInfo.bootEntry;
    cpu.scu.registerFile[Binary::STATUS].ui = 0b10;  // enable exl at boot <- has to be done by hardware
}

void Hardware::Machine::loadKernel(const std::vector<std::string>& kernelArgs ) {
    trapEntry = KernelElf::trapEntry;
    Word at = KernelElf::textStart;

    for (uint8_t byte : KernelElf::kernelText) {
        memory.setByte(at, byte, cpu.tlb, 0);
        ++at;
    }

    at = KernelElf::dataStart;
    for (uint8_t byte : KernelElf::kernelData) {
        memory.setByte(at, byte, cpu.tlb, 0);
        ++at;
    }

    memory.setWord(KernelElf::argc, kernelArgs.size(), cpu.tlb, 0);

    for (Word i = 0; i < kernelArgs.size(); ++i) {
        Word indirectPtr = KernelElf::argv + 64 * i;    // argv[i] = *(argv + i)
        for (Word j = 0; j < kernelArgs[i].size(); ++j) memory.setByte(indirectPtr + j, kernelArgs[i][j], cpu.tlb, 0 ); // argv[i][j]
    }

    cpu.programCounter = KernelElf::bootEntry;
    cpu.scu.registerFile[Binary::STATUS].ui = 0b10;

}

// Runs a single cycle. Since the interrupt system is clock-based and not instr count based, interrupts are disabled
// Use a Hardware::SendInterrupt object if you want to manually call an interrupt
void Hardware::Machine::step() {
    cpu.cycle();
}


// Runs with an Interrupt Device.
void Hardware::Machine::run(instrDebugHook hook, std::chrono::milliseconds IDduration ) {
    if (cpu.interDev) cpu.interDev->start(IDduration);
    while (!killed) {
        step();
        if (hook) hook(*this);
    }
    if (cpu.interDev) cpu.interDev->stop();
}