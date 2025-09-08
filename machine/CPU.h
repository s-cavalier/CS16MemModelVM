#ifndef __CPU_H__
#define __CPU_H__
#include "Processors.h"
#include <unordered_map>
#include <atomic>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <mutex>

namespace Hardware {

    struct Trap final : std::exception {
        enum ExceptionCode : unsigned char {
            INTERRUPT = 0,   // Interrupt (hardware)

            TLB_MOD = 1,
            TLB_L = 2,
            TLB_S = 3,

            ADDRL = 4,   // Address error on load or fetch
            ADDRS = 5,   // Address error on store
            SYSCALL = 8,   // SYSCALL instruction
            BREAK = 9,   // BREAK instruction
            RI = 10,  // Reserved instruction
            CP_UNUSABLE = 11,  // Coprocessor unusable
            OVERFLOW = 12,  // Arithmetic overflow
            TRAP = 13,  // TRAP instruction (e.g., TEQ + trap conditions)
            FP_EXC = 15,  // Floating-point exception
            WATCH = 23,  // Watchpoint reference
            // Other values are reserved or implementation-specific
        };

        ExceptionCode exceptionCode;
        Word badAddr;
        explicit Trap(ExceptionCode exceptionCode, Word badAddr = 0) : exceptionCode(exceptionCode), badAddr(badAddr) {}
    };

    // ABC Interrupt Device to try out different ID's
    struct InterruptDevice {
        virtual void start(std::chrono::milliseconds quanta) = 0;
        virtual void stop() = 0;
        virtual bool poll() = 0;
        virtual ~InterruptDevice() = default;
    };

    // Could try to use a spin-system instead if we want microsecond interrupts
    class WallClock final : public InterruptDevice {
        std::chrono::milliseconds duration;
        std::thread poller;
        std::condition_variable cv; // CV / Mutex for interrupting the sleeping thread on destructor or stop()
        std::mutex mx;
        std::atomic_bool IDactive, polling; // IDactive is the producer/consumer atomic for sending an interrupt, polling is the producer/consumer atomic for killing the thread
        
        void run();

    public:
        WallClock();
        ~WallClock();

        void start(std::chrono::milliseconds quanta) override;
        void stop() override;

        bool poll() override;
    };

    class TickClock final : public InterruptDevice {
        uint64_t tickGoal;
        uint64_t tickCount;
        bool ticking;

    public:
        TickClock();

        void start(std::chrono::milliseconds quanta) override;
        void stop() override;

        bool poll() override;
    };

    // Can be used in Hardware::Machine::step()
    class SendInterrupt final : public InterruptDevice {
        void start(std::chrono::milliseconds) override {}
        void stop() override {}
        bool poll() override { return true; }
    };


    struct HiLoRegisters { Word hi; Word lo; };

    class CPU {
        Machine& machine;
        reg32_t registerFile[32];
        HiLoRegisters hiLo{};
        Word programCounter;
        std::unordered_map<Word, std::unique_ptr<Instruction>> instructionCache;
    
    public:
        CPU(Machine& machine);       // maybe cache later down the road
        inline const reg32_t& readRegister(const Byte& reg) const { return registerFile[reg]; }
        inline reg32_t& accessRegister(const Byte& reg) { return registerFile[reg]; }
        inline const Word& readProgramCounter() const { return programCounter; }
        inline Word& accessProgramCounter() { return programCounter; }
        inline HiLoRegisters readHiLo() const { return hiLo; }

        void cycle( InterruptDevice* device );

        std::unique_ptr<Instruction> decode(const Word& bin_instr);

    };
}

#endif