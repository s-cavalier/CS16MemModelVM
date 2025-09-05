#ifndef __VIRTUAL_MEMORY_H__
#define __VIRTUAL_MEMORY_H__
#include "kstl/Bitset.h"
#include "kstl/Optional.h"
#include "kstl/Array.h"
#include "kstl/UniquePtr.h"

namespace kernel {

    using addr_t = uint32_t;

    constexpr size_t MEM_AVAIL = 256_mb;
    constexpr size_t PAGE_SIZE = 4096;
    constexpr size_t NUM_PAGES = MEM_AVAIL / PAGE_SIZE;
    constexpr size_t BIT_OFFSET = 12;

    class MemoryManager {
        ministl::bitset<NUM_PAGES> reservedPages;
        size_t kernelReservedBoundary;

        MemoryManager();
    public:

        static MemoryManager& instance() {
            static MemoryManager inst;
            return inst;
        }

        size_t reserveFrame();
        void freeFrame(addr_t paddr);
    };

    struct TLBEntry {
        uint32_t hi{};
        uint32_t lo{};

        TLBEntry() = default;

        enum Flags : unsigned char {
            GLOBAL = 1,
            DIRTY = 2,
            VALID = 4
        };

        // TODO: Add in CC
        // OR together TLBEntry::flags into the flags value
        TLBEntry(uint32_t vpn, uint32_t pfn, unsigned char asid, unsigned char flags);

        void writeRandom() const;
        void writeIndexed(unsigned char idx) const;
        static TLBEntry fromRead(unsigned char idx);
        static ministl::optional<TLBEntry> fromProbe(uint32_t vpn, bool global, unsigned char asid);

        // Generate from Addresses ( just calls return TLBEntry(vaddr >> 12, paddr >> 12, asid, flags) )
        inline static TLBEntry fromAddresses(uint32_t vaddr, uint32_t paddr, unsigned char asid, unsigned char flags) { return TLBEntry(vaddr >> 12, paddr >> 12, asid, flags); }

    };

    // -------------------------------------------------------------------------------
    // Primary PageTable ABC all PageTable implementations should inherit and abide to
    // -------------------------------------------------------------------------------
    // Each PageTable should only request resources (from MemoryManager::instance()) in it's constructor since the AddressSpace won't be sure 
    // how to initialize a certain PT type. Otherwise, all resources given will come through the owning AddressSpace.
    struct PageTable {
        // Based off of https://pages.cs.wisc.edu/~remzi/OSTEP/vm-paging.pdf
        struct Entry {
            uint32_t pfn{};
            bool global{};
            bool writable{};
            bool user{};
            bool present{};

            Entry() = default;

            enum Flags : unsigned char {
                PRESENT = 1,
                WRITABLE = 2,
                USER = 4,
                // implement PWT, PCD, A, D, PAT later
                GLOBAL = 128
            };

            // OR PTE::FLags together in flags field
            Entry(uint32_t pfn, unsigned char flags);

            // Implicitly marks returned tlbe as valid
            TLBEntry toTLBEntry(uint32_t vpn, unsigned char asid) const;

            // Just calls return PageTableEntry(paddr >> 12, flags). OR PTE::Flags together in flags field
            inline static Entry fromPAddress(uint32_t paddr, unsigned char flags) { return Entry(paddr >> 12, flags); }

        };


        virtual ~PageTable() = default;

        virtual ministl::optional<Entry> walkTable(uint32_t vaddr) const = 0;
        virtual bool mapPTE(const Entry& pte, uint32_t vpn) = 0;

        // Overload that sort of forward the arguments to something like mapPTE( entry(pfn, flags), vpn )
        inline bool mapPTEfa(uint32_t pfn, unsigned char flags, uint32_t vpn) { return mapPTE( kernel::PageTable::Entry(pfn, flags), vpn ); }
        
    };

    static constexpr uint32_t KHEAP_SIZE = 256_kb;
    static constexpr uint32_t KHEAP_PAGES = KHEAP_SIZE / 4096;

    static constexpr uint32_t KHEAP_BOUNDARY = 0xC0000000;

    // Doesn't need a destructor since it won't get cleaned up until program end
    class KernelPageTable : public PageTable {
        ministl::array<PageTable::Entry, KHEAP_PAGES> pt;        
        KernelPageTable();

    public:
        static KernelPageTable& instance() {
            static KernelPageTable kpt;
            return kpt;
        }
        

        ministl::optional<Entry> walkTable(uint32_t vaddr) const override;
        bool mapPTE(const Entry &pte, uint32_t vpn) override;
    };

    class AddressSpace {
        ministl::unique_ptr<PageTable> _pageTable;
        unsigned char _asid;
        
        // Private constructor for the kernel
        AddressSpace() : _pageTable( ministl::unique_ptr<PageTable>( &KernelPageTable::instance() )), _asid(0) {} 
        friend class PCB;

    public:
        AddressSpace(ministl::unique_ptr<PageTable> pageTable, unsigned char asid);
        
        TLBEntry translate(uint32_t vaddr);

    };

}





#endif