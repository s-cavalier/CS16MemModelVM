#include "Memory.h"
#include "BinaryUtils.h"
#include "Processors.h"
#include <cstring>

#include <iostream>

Hardware::TLBEntry::TLBEntry(Word hi, Word lo) {
    // Based on https://pages.cs.wisc.edu/~remzi/OSTEP/vm-tlbs.pdf
    vpn = (hi >> 12);
    global = bool(hi & (1 << 11));
    asid = Byte(hi & 0xFF);
    pfn = (lo >> 6);
    cc = Byte( (lo >> 3) & 0b111 );
    dirty = bool(lo & 0b100);
    valid = bool(lo & 0b10);
}

std::pair<Word, Word> Hardware::TLBEntry::hiLoPair() const {
    return {
        (vpn << 12) |
        (Word(global) << 11) |
        asid,

        (pfn << 6) |
        (cc << 3) |
        (Word(dirty) << 2) |
        (Word(valid) << 1)
    };
}

Hardware::TLBEntry Hardware::TLB::lookup(Word vaddr, LookupType type) const {
    Word vpn = vaddr >> 12;

    for (Byte i = 0; i < TLB_ENTRIES; ++i) {
        const auto& entry = tlbEntries[i];
        
        if (vpn != entry.vpn || !entry.valid) continue;
        return entry;

    }

    throw Trap(type == LOAD ? Trap::TLB_L : Trap::TLB_S, vaddr);
}

Word Hardware::TLB::translate( Word vaddr, LookupType type ) const {
    if ( vaddr >= 0x80000000 && vaddr < 0xC0000000 ) return vaddr - 0x80000000;

    Word offset = vaddr & 0xFFF;
    TLBEntry tlbe = lookup( vaddr, type );

    return (tlbe.pfn << 12) | offset;
}

Hardware::Memory::Memory() : physMem(new Byte[PHYS_MEM_SIZE]) {}

Byte Hardware::Memory::getByte(Word addr, const TLB& tlb) const {
    Word paddr = tlb.translate(addr, TLB::LOAD);
    
    return physMem[paddr];
}

HalfWord Hardware::Memory::getHalfWord(Word addr, const TLB& tlb) const {
    Word paddr = tlb.translate(addr, TLB::LOAD);

    assert( (paddr % 2 == 0) && "Invalid load 2 -> switch to a trap later" );
    Byte* loc = physMem.get() + paddr;

    return HalfWord(loc[0] << 8) | HalfWord(loc[1]);
}

Word Hardware::Memory::getWord(Word addr, const TLB& tlb) const {
    Word paddr = tlb.translate(addr, TLB::LOAD);

    assert( (paddr % 4 == 0) && "Invalid load 4 -> switch to a trap later" );
    Byte* loc = physMem.get() + paddr;

    return Binary::loadBigEndian( loc );
}

void Hardware::Memory::setByte(Word addr, Byte byte, const TLB& tlb) {
    Word paddr = tlb.translate(addr, TLB::STORE);
    
    physMem[paddr] = byte;
}

void Hardware::Memory::setHalfWord(Word addr, HalfWord halfword, const TLB& tlb) {
    Word paddr = tlb.translate(addr, TLB::STORE);

    assert( (paddr % 2 == 0) && "Invalid store 2 -> switch to a trap later" );
    Byte* loc = physMem.get() + paddr;

    loc[0] = (halfword >> 8);
    loc[1] = (halfword & 0xFF);
}

void Hardware::Memory::setWord(Word addr, Word word, const TLB& tlb) {
    Word paddr = tlb.translate(addr, TLB::STORE);

    assert( (paddr % 4 == 0) && "Invalid store 4 -> switch to a trap later" );
    Byte* loc = physMem.get() + paddr;

    loc[0] = word >> 24;
    loc[1] = (word >> 16) & 0xFF;
    loc[2] = (word >> 8) & 0xFF;
    loc[3] = word & 0xFF;
}

float Hardware::Memory::getSingle(Word addr, const TLB& tlb) const {
    Word tmpw = getWord(addr, tlb);
    float tmpf = 0;
    memcpy(&tmpf, &tmpw, 4);
    return tmpf;
}

void Hardware::Memory::setSingle(Word addr, float single, const TLB& tlb) {
    Word tmpw = 0;
    memcpy(&tmpw, &single, 4);
    setWord(addr, tmpw, tlb);
}