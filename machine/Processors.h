#ifndef __PROCESSORS_H__
#define __PROCESSORS_H__
#include "Memory.h"
#include "LRUCache.h"

#include <memory>
#include <unordered_map>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <cassert>

using Word = unsigned int;
using HalfWord = unsigned short;
using Byte = unsigned char;
using Double = double;
using Single = float;

namespace Hardware {

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

    struct Instruction;
    struct Core;

    union reg32_t {
        Word ui;
        int i;
        float f;
    };

    class Processor {
        Core* _core;  // back reference
        
    
    public:
        Core& core() { return *_core; }
        const Core& core() const { return *_core; }

        alignas(8) reg32_t registerFile[32];    // aligned for FPU to be able to reinterpret_cast pairs of registers into double, switch to alignas(16) if implementing quad-prec

        Processor(Core& core);


        virtual std::unique_ptr<Instruction> decode(Word bin_instr) = 0;

        virtual ~Processor() = default;
    };

    class FloatingPointUnit : public Processor {
        bool FPcond;

    public:
        FloatingPointUnit(Core& core);
        std::unique_ptr<Instruction> decode(Word bin_instr) override;

        inline const Double& getDouble(Byte reg) const { 
            using DoubleAlias __attribute__((__may_alias__)) = Double;
            assert(!(reg & 1)); 
            return *reinterpret_cast<const DoubleAlias*>(&registerFile[reg].f); 
        }

        inline Double& getDouble(Byte reg) { 
            using DoubleAlias __attribute__((__may_alias__)) = Double;
            assert(!(reg & 1)); 
            return *reinterpret_cast<DoubleAlias*>(&registerFile[reg].f); 
        }
    };

    struct SystemControlUnit : public Processor {
        SystemControlUnit(Core& core);

        inline void setEPC(Word pc) { registerFile[14].ui = pc; }
        inline void setCause(Byte exceptionCode) { registerFile[13].ui = Word(exceptionCode) << 2; }
        inline void setBadVAddr(Word vaddr) { registerFile[8].ui = vaddr; }
        inline void setEXL(bool enable) { if (enable) registerFile[12].ui |= 0b10; else registerFile[12].ui &= ~Word(0b10); }

        std::unique_ptr<Instruction> decode(Word bin_instr) override;

    };

    struct HiLoRegisters { Word hi; Word lo; };

    struct IntegerUnit : public Processor {
        HiLoRegisters hiLo{};

        IntegerUnit(Core& core);       // maybe cache later down the road

        std::unique_ptr<Instruction> decode(Word bin_instr) override;
    };

    struct Trap {
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
    
    struct Machine;

    class Core {
        Machine* _machine; // back reference

    public:
        Word programCounter;
        IntegerUnit iu;
        FloatingPointUnit fpu;
        SystemControlUnit scu;
        std::unique_ptr<InterruptDevice> interDev;
        LRUCache<Word, std::unique_ptr<Instruction>, 32 * 1024> instructionCache;

        Core( Machine& machine );
        Machine& machine() { return *_machine; }
        const Machine& machine() const { return *_machine; }

        void cycle();
    };

};


#endif