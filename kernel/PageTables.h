#ifndef __PAGE_TABLES_H__
#define __PAGE_TABLES_H__
#include "VirtualMemory.h"
#include "kstl/Vector.h"

namespace kernel {

    static constexpr uint32_t STACK_LIMIT   = 0x80000000;
    static constexpr uint32_t DYNAMIC_START = 0x10008000;
    static constexpr uint32_t STATIC_START  = 0x10000000;
    static constexpr uint32_t TEXT_START    = 0x00400000;
    
    class SegmentedPageTable : public PageTable {
        ministl::vector<PageTable::Entry> stackPages;
        ministl::vector<PageTable::Entry> dynamicPages;
        ministl::vector<PageTable::Entry> staticPages;
        ministl::vector<PageTable::Entry> textPages;

    public:
        SegmentedPageTable(size_t initText, size_t initStack = 2, size_t initStatic = 4, size_t initDynamic = 0);

        ministl::optional<Entry> walkTable(uint32_t vaddr) const override;
        bool mapPTE(const Entry &pte, uint32_t vpn) override;

        ~SegmentedPageTable() override;
    };


}

#endif