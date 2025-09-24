#ifndef __MEMORY_H__
#define __MEMORY_H__
#include <array>
#include <memory>

using Byte = unsigned char;
using HalfWord = unsigned short;
using Word = unsigned int;

#define PAGE_SIZE 4096UL
#define PHYS_MEM_SIZE 5242880UL
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
        
        TLBEntry lookup(Word vaddr, LookupType type, Byte asid) const;

        inline TLBEntry& operator[](size_t idx) { return tlbEntries.at(idx); }
        inline const TLBEntry& operator[](size_t idx) const { return tlbEntries.at(idx); }

        Word translate( Word vaddr, LookupType type, Byte asid ) const;
    };

    class Memory {
        std::unique_ptr<Byte[]> physMem;

    public:
        Memory();

        Word getWord(Word addr, const TLB& tlb, Byte asid) const;
        HalfWord getHalfWord(Word addr, const TLB& tlb, Byte asid) const;
        Byte getByte(Word addr, const TLB& tlb, Byte asid) const;

        void setWord(Word addr, Word word, const TLB& tlb, Byte asid);
        void setHalfWord(Word addr, HalfWord halfword, const TLB& tlb, Byte asid);
        void setByte(Word addr, Byte byte, const TLB& tlb, Byte asid);

        float getSingle(Word addr, const TLB& tlb, Byte asid) const;
        double getDouble(Word addr, const TLB& tlb, Byte asid) const;

        void setSingle(Word addr, float single, const TLB& tlb, Byte asid);
        void setDouble(Word addr, double dble, const TLB& tlb, Byte asid);
    };

    class VirtualMemory {

    };

};

#endif