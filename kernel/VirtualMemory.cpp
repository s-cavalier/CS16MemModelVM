#include "VirtualMemory.h"


extern "C" char _end[];

kernel::MemoryManager::MemoryManager() {
    kernelReservedBoundary = ((size_t(_end) - 0x80000000) + PAGE_SIZE - 1) / PAGE_SIZE; // round up to closet page boundary in kernel image
}

size_t kernel::MemoryManager::reserveFrame() {

    for (size_t i = kernelReservedBoundary; i < reservedPages.size(); ++i) {
        if (reservedPages[i]) continue; // reserved if bit[i] == 1
        reservedPages.set(i);
        return i << BIT_OFFSET;
    }

    assert(false && "Ran out of available frames!");
    return -1;
}

void kernel::MemoryManager::freeFrame(addr_t paddr) {
    reservedPages.set( paddr >> BIT_OFFSET, false );
}

kernel::TLBEntry::TLBEntry(uint32_t vpn, uint32_t pfn, unsigned char asid, unsigned char flags) {
    #define SHIFT_FLAG(flag_t, shift_amt) uint32_t(bool(flags & flag_t)) << shift_amt 

    hi = (vpn << 12) | ( SHIFT_FLAG(GLOBAL, 11) ) | asid;
    lo = (pfn << 6) | ( SHIFT_FLAG(DIRTY, 2) ) | ( SHIFT_FLAG(VALID, 1) );
}

void kernel::TLBEntry::writeRandom() const {
    __asm__ volatile(
        "mtc0 %0, $10\n"
        "mtc0 %1, $2\n"
        "tlbwr\n"
        : 
        : "r"(hi), "r"(lo)
        : "memory"
    );
}

void kernel::TLBEntry::writeIndexed(unsigned char idx) const {
    __asm__ volatile(
        "mtc0 %0, $10\n"
        "mtc0 %1, $2\n"
        "mtc0 %2, $0\n"
        "tlbwi\n"
        :
        : "r"(hi), "r"(lo), "r"(uint32_t(idx))
        : "memory"
    );
}

kernel::TLBEntry kernel::TLBEntry::fromRead(unsigned char idx) {
    TLBEntry tlbe{};
    __asm__ volatile(
        "mtc0 %2, $0\n"
        "tlbr\n"
        "mfc0 %0, $10\n"
        "mfc0 %1, $2\n"
        : "=r"(tlbe.hi), "=r"(tlbe.lo)
        : "r"(uint32_t(idx))
        : "memory"
    );
    
    return tlbe;
}

ministl::optional<kernel::TLBEntry> kernel::TLBEntry::fromProbe(uint32_t vpn, bool global, unsigned char asid) {
    size_t indexReg = 0;
    uint32_t entryHi = (vpn << 12) | ( uint32_t(global) << 11 ) | asid;

    __asm__ volatile(
        "mtc0 %1, $10\n"
        "tlbp\n"
        "mfc0 %0, $0\n"
        : "=r"(indexReg)
        : "r"(entryHi)
        : "memory"
    );

    if (indexReg == 0x80000000) return ministl::nullopt;

    return fromRead(indexReg);
}

kernel::PageTable::Entry::Entry(uint32_t pfn, unsigned char flags) 
: pfn( pfn << 12 ), global(flags & GLOBAL), writable(flags & WRITABLE), user(flags & USER), present(flags & USER)
{}

kernel::TLBEntry kernel::PageTable::Entry::toTLBEntry(uint32_t vpn, unsigned char asid) const {
    #define CONVERT_FLAG(flag, offset) (uint32_t(flag) << offset) 

    return TLBEntry(
        vpn,
        pfn,
        asid,
        TLBEntry::VALID | (uint32_t)(global)
    );
}

kernel::KernelPageTable::KernelPageTable() {
    for (size_t i = 0; i < pt.capacity(); ++i) {
        pt[i].global = true;
        pt[i].present = true; // Change later when doing swapping
        pt[i].writable = true; 
        pt[i].pfn = MemoryManager::instance().reserveFrame() >> 12;
    }
}

ministl::optional<kernel::PageTable::Entry> kernel::KernelPageTable::walkTable(uint32_t vaddr) const {
    assert(vaddr >= KHEAP_BOUNDARY);
    
    size_t vpn = (vaddr - KHEAP_BOUNDARY) >> 12;
    assert(vpn < KHEAP_PAGES);

    return pt[vpn];
}

bool kernel::KernelPageTable::mapPTE(const Entry &pte, uint32_t vpn) {
    constexpr uint32_t VPN_KHEAP_BOUNDARY = KHEAP_BOUNDARY >> 12;
    assert(vpn >= VPN_KHEAP_BOUNDARY);
    size_t normalized_vpn = vpn - VPN_KHEAP_BOUNDARY;
    assert(normalized_vpn < KHEAP_PAGES);

    MemoryManager::instance().freeFrame( pt[normalized_vpn].pfn << 12 );
    (pt[ normalized_vpn ] = pte).global = true; // all kernel pages should be glob
    return true;
} 

ministl::bitset<255> kernel::AddressSpace::reservedASIDs;

kernel::AddressSpace::AddressSpace(kernelInit_t, KernelPageTable& kpt) : _pageTable( &kpt ), _asid(KERNEL_ASID) {}

kernel::AddressSpace::AddressSpace(ministl::unique_ptr<PageTable> pageTable) : _pageTable( ministl::move(pageTable) ), _asid(0) {
    for (size_t i = 0; i < reservedASIDs.size(); ++i) {
        if (reservedASIDs[i]) continue;
        reservedASIDs.set(i);
        _asid = i;
        return;
    }

    // Panic until more infra built
    assert(false && "No ASIDs available");
}

kernel::TLBEntry kernel::AddressSpace::translate(uint32_t vaddr) {
    auto res = _pageTable->walkTable(vaddr);

    if (res && res->present) return res->toTLBEntry(vaddr >> 12, _asid);

    assert(false && "not implemented yet");
    return {};
}

bool kernel::AddressSpace::updateBrk(uint32_t vaddr) {
    if (vaddr < BRK_DEFAULT || vaddr >= HEAP_LIMIT) return false;
    uint32_t pageBoundary = ((vaddr - BRK_DEFAULT) >> 12) + 1;
    if (pageBoundary <= brk) return true;

    static constexpr uint32_t vpn_start = BRK_DEFAULT >> 12;
    for (size_t i = brk; i < pageBoundary; ++i) {

        PageTable::Entry pte{};
        pte.pfn = MemoryManager::instance().reserveFrame() >> 12;;
        pte.global = false;
        pte.writable = true;
        pte.user = true;
        pte.present = true;

        _pageTable->mapPTE(pte,  vpn_start + i);
    }

    brk = pageBoundary;

    return true;
}

// Free asid
kernel::AddressSpace::~AddressSpace() {
    reservedASIDs.set(_asid, false);
}


