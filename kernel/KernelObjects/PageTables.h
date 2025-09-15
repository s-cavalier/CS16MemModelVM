#ifndef __PAGE_TABLES_H__
#define __PAGE_TABLES_H__
#include "../SharedResources/VirtualMemory.h"
#include "../kstl/Vector.h"
#include "../kstl/UnorderedMap.h"

namespace kernel {

    static constexpr uint32_t STACK_LIMIT   = 0x80000000;
    static constexpr uint32_t DYNAMIC_START = 0x10008000;
    static constexpr uint32_t STATIC_START  = 0x10000000;
    static constexpr uint32_t TEXT_START    = 0x00400000;

    static constexpr uint32_t STACK_LIMIT_VPN   = STACK_LIMIT >> 12;
    static constexpr uint32_t DYNAMIC_START_VPN = DYNAMIC_START >> 12;
    static constexpr uint32_t STATIC_START_VPN  = STATIC_START >> 12;
    static constexpr uint32_t TEXT_START_VPN    = TEXT_START >> 12;
    
    class SegmentedPageTable : public PageTable {
        ministl::vector<PageTable::Entry> stackPages;
        ministl::vector<PageTable::Entry> dynamicPages;
        ministl::vector<PageTable::Entry> staticPages;
        ministl::vector<PageTable::Entry> textPages;

    public:
        SegmentedPageTable(size_t initText, size_t initStatic = 4, size_t initStack = 2, size_t initDynamic = 0);

        ministl::unique_ptr<Iterator> getIterator() const override { return ministl::unique_ptr<Iterator>(nullptr); } // Temporary stub 
        ministl::optional<Entry> walkTable(uint32_t vaddr) const override;
        bool mapPTE(const Entry &pte, uint32_t vpn) override;

        ~SegmentedPageTable() override;
    };

    class HashPageTable : public PageTable {
        ministl::unordered_map<uint32_t, Entry> table;
    
    public:

        class iterator : public Iterator {
            ministl::unordered_map<uint32_t, Entry>::const_iterator it, end;
            iterator() = default;

        public:
            void operator++() override;
            ministl::optional<pagePair> operator*() override;
            
            ~iterator() = default;

            friend class HashPageTable;
        };

        friend class iterator;

        HashPageTable(size_t initText, size_t initStatic = 4, size_t initStack = 2, size_t initDynamic = 0);
        HashPageTable(ministl::unique_ptr<Iterator> it);

        ministl::unique_ptr<Iterator> getIterator() const override;
        ministl::optional<Entry> walkTable(uint32_t vaddr) const override;
        bool mapPTE(const Entry &pte, uint32_t vpn) override;

        ~HashPageTable() override;
    };


}

#endif