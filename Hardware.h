#ifndef __HARDWARE_H__
#define __HARDWARE_H__
#include <unordered_map>
#include <memory>
#include <vector>

// TODO: Implement memory access guards

using Word = unsigned int;
using HalfWord = unsigned short;
using Byte = unsigned char;

namespace Hardware {
    struct Instruction;

    class Memory {
        std::unordered_map<Word, char> RAM;
    
    public:
        class Iterator {
            std::unordered_map<Word, char>::const_iterator* it;

        public:
            using value_type = const std::pair<const Word, char>;
            using reference = value_type&;
            using pointer = value_type*;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::forward_iterator_tag;

            Iterator(const std::unordered_map<Word, char>::const_iterator& src);
            ~Iterator();

            reference operator*() const;
            pointer   operator->() const;

            Iterator& operator++();
            Iterator operator++(int);

            bool operator==(const Iterator& other) const;
            bool operator!=(const Iterator& other) const;
        };

        struct boundRegisters {
            Word textBound;
            Word staticBound;
            Word dynamicBound;
            Word stackBound;
        };

        boundRegisters memoryBounds;

        Iterator begin() const;
        Iterator end() const;

        Memory();
        Memory(const boundRegisters& bounds);

        Word getWord(const Word& addr) const;
        HalfWord getHalfWord(const Word& addr) const;
        Byte getByte(const Word& addr) const;

        void setWord(const Word& addr, const Word& word);
        void setHalfWord(const Word& addr, const HalfWord& halfword);
        void setByte(const Word& addr, const Byte& byte);

    };

    class Machine {
        Word programCounter;
        Word hi_lo[2];
        int registerFile[32];
        std::unordered_map<Word, std::unique_ptr<Instruction>> instructionCache;
        Memory RAM;

    public:
        Machine();

        const Word& readProgramCounter() const;
        const int& readRegister(const Byte& reg) const;
        const Memory& readMemory() const;
        bool killed;

        void loadInstructions(const std::vector<Word>& instructions);

        void runInstruction();

        // Just calls a loop on runInstruction until kill flag is set
        void run();
    };

    struct Instruction {
        virtual void run() = 0;
    };

    std::unique_ptr<Instruction> instructionFactory(const Word& binary_instruction, Word& programCounter, int* registerFile, Hardware::Memory& RAM, bool& kill_flag);

};

#endif