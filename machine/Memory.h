#ifndef __MEMORY_H__
#define __MEMORY_H__
#include <array>
#include <memory>

using Byte = unsigned char;
using HalfWord = unsigned short;
using Word = unsigned int;

#define PAGE_SIZE 4096UL
#define PHYS_MEM_SIZE 268435456ULL
#define TLB_ENTRIES 64


namespace Hardware {
    
    struct TLBEntry {
        Word vpn = 0;
        Word pfn = 0;
        bool global = false;
        bool dirty = false;
        bool valid = false;
        Byte asid = 0;
        Byte cc = 0;

        TLBEntry() = default;
        TLBEntry(Word hi, Word lo); // Construct from hi / lo

        std::pair<Word, Word> hiLoPair() const;
    };

    class TLB {
        std::array<TLBEntry, TLB_ENTRIES> tlbEntries;

    public:
        TLB() = default;

        enum LookupType : Byte {
            LOAD,
            STORE,
        };
        
        TLBEntry lookup(Word vaddr, LookupType type) const;

        inline TLBEntry& operator[](size_t idx) { return tlbEntries.at(idx); }
        inline const TLBEntry& operator[](size_t idx) const { return tlbEntries.at(idx); }
    };

    class Memory {
        std::unique_ptr<Byte[]> physMem;
        TLB tlb;


        Word runTLB( Word vaddr, TLB::LookupType type ) const;
    public:
        Memory();
        TLB& accessTLB() { return tlb; }
        const TLB& readTLB() const { return tlb; }

        Word getWord(Word addr) const;
        HalfWord getHalfWord(Word addr) const;
        Byte getByte(Word addr) const;

        void setWord(Word addr, Word word);
        void setHalfWord(Word addr, HalfWord halfword);
        void setByte(Word addr, Byte byte);

        float getSingle(Word addr) const;
        double getDouble(Word addr) const;

        void setSingle(Word addr, float single);
        void setDouble(Word addr, double dble);
    };

    class VirtualMemory {

    };

};

#endif