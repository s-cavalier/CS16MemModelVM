#ifndef __VIRTUAL_MEMORY_H__
#define __VIRTUAL_MEMORY_H__
#include "../kstl/Bitset.h"
#include "../kstl/Optional.h"
#include "../kstl/Array.h"
#include "../kstl/UniquePtr.h"

namespace kernel {

    using addr_t = uint32_t;

    constexpr size_t MEM_AVAIL = 256_mb;
    constexpr size_t PAGE_SIZE = 4096;
    constexpr size_t NUM_PAGES = MEM_AVAIL / PAGE_SIZE;
    constexpr size_t BIT_OFFSET = 12;

    class MemoryManager {
        ministl::bitset<NUM_PAGES> reservedPages;
        size_t kernelReservedBoundary;

        friend class SharedResources;
        MemoryManager();

    public:

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
        static void invalidate(unsigned char idx);
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

            // Copy memory contents of the physical frame to another physical frame
            void copyMemoryTo(uint32_t pfn) const;
        };

        // Abstract Iterator interface so that each PageTable can implement a constuctor with unique_ptr<Iterator> as the abstraction over the iteration
        struct Iterator {
            virtual ~Iterator() = default;
            virtual void operator++() = 0;

            struct pagePair { uint32_t vpn; Entry pte; };
            virtual ministl::optional<pagePair> operator*() = 0;
        };

        virtual ~PageTable() = default;

        virtual ministl::unique_ptr<Iterator> getIterator() const = 0;
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

        friend class ProcessManager;

        // Should only be accessible through the ProcessManager

    public:
        ministl::unique_ptr<Iterator> getIterator() const override { return ministl::unique_ptr<Iterator>(nullptr);  } // This shouldn't be accessed 
        ministl::optional<Entry> walkTable(uint32_t vaddr) const override;
        bool mapPTE(const Entry &pte, uint32_t vpn) override;
    };

    static constexpr uint32_t BRK_DEFAULT = 0x10008000;
    static constexpr uint32_t HEAP_LIMIT = BRK_DEFAULT + (1 * 1024 * 1024);

    // Used for kernel initialization constructor routines, namely AddrSpace(kinit, ...) and PCB(kinit, ...)
    struct kernelInit_t {
        explicit constexpr kernelInit_t() = default;
    };

    inline constexpr kernelInit_t kernelInitalizer{};

    class AddressSpace {

        // the 256th bit is reserved for kernel_asid = 0xFF
        static ministl::bitset<255> reservedASIDs; 

        ministl::unique_ptr<PageTable> _pageTable;
        size_t brk;
        unsigned char _asid;

        AddressSpace(kernelInit_t, KernelPageTable& kpt);
        AddressSpace(ministl::unique_ptr<PageTable> pageTable);

        // AddressSpace initialization should go strictly through the ProcessManager/PCB
        friend class ProcessManager;
        friend class PCB;

    public:
        static constexpr unsigned char KERNEL_ASID = 0xFF;

        AddressSpace(const AddressSpace& other) = delete;
        AddressSpace(AddressSpace&& other) = delete;
        AddressSpace& operator=(const AddressSpace& other) = delete;
        AddressSpace& operator=(AddressSpace&& other) = delete; // Maybe make these later

        unsigned char getASID() const { return _asid; }
        
        TLBEntry translate(uint32_t vaddr);
        bool updateBrk(uint32_t vaddr);

        ~AddressSpace();
    };

}





#endif