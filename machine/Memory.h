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

        Word translate( Word vaddr, LookupType type ) const;
    };

    class Memory {
        std::unique_ptr<Byte[]> physMem;

    public:
        Memory();

        

        Word getWord(Word addr, const TLB& tlb) const;
        HalfWord getHalfWord(Word addr, const TLB& tlb) const;
        Byte getByte(Word addr, const TLB& tlb) const;

        void setWord(Word addr, Word word, const TLB& tlb);
        void setHalfWord(Word addr, HalfWord halfword, const TLB& tlb);
        void setByte(Word addr, Byte byte, const TLB& tlb);

        float getSingle(Word addr, const TLB& tlb) const;
        double getDouble(Word addr, const TLB& tlb) const;

        void setSingle(Word addr, float single, const TLB& tlb);
        void setDouble(Word addr, double dble, const TLB& tlb);
    };

    class VirtualMemory {

    };

};

#endif