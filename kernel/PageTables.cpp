#include "PageTables.h"


kernel::SegmentedPageTable::SegmentedPageTable(size_t initText, size_t initStatic, size_t initStack, size_t initDynamic) :
stackPages(initStack), dynamicPages(initDynamic), staticPages(initStatic), textPages(initText) {

    for (size_t i = 0; i < stackPages.size(); ++i) {
        stackPages[i].pfn = MemoryManager::instance().reserveFrame() >> 12;
        stackPages[i].global = false;
        stackPages[i].present = true;
        stackPages[i].user = true;
        stackPages[i].writable = true;
    }

    for (size_t i = 0; i < dynamicPages.size(); ++i) {
        dynamicPages[i].pfn = MemoryManager::instance().reserveFrame() >> 12;
        dynamicPages[i].global = false;
        dynamicPages[i].present = true;
        dynamicPages[i].user = true;
        dynamicPages[i].writable = true;
    }

    for (size_t i = 0; i < staticPages.size(); ++i) {
        staticPages[i].pfn = MemoryManager::instance().reserveFrame() >> 12;
        staticPages[i].global = false;
        staticPages[i].present = true;
        staticPages[i].user = true;
        staticPages[i].writable = true;
    }

    for (size_t i = 0; i < textPages.size(); ++i) {
        textPages[i].pfn = MemoryManager::instance().reserveFrame() >> 12;
        textPages[i].global = false;
        textPages[i].present = true;
        textPages[i].user = true;
        textPages[i].writable = false;
    }

}

ministl::optional<kernel::PageTable::Entry> kernel::SegmentedPageTable::walkTable(uint32_t vaddr) const {
    size_t stackBoundary = STACK_LIMIT - ( stackPages.size() << 12 );
    if (vaddr < STACK_LIMIT && vaddr >= stackBoundary) {
        // since the stack grows down, the calculation for index into the vector is (0x80000000 - (vaddr & 0xFFF) ) >> 12 == 0x80000 - (vaddr >> 12)
        size_t index = 0x80000 - (vaddr >> 12);
        return stackPages[index];
    }

    size_t dynamicBoundary = DYNAMIC_START + ( dynamicPages.size() << 12 );
    if (vaddr < dynamicBoundary && vaddr >= DYNAMIC_START) {
        size_t index = (vaddr - DYNAMIC_START) >> 12;
        return dynamicPages[index];
    }

    size_t staticBoundary = STATIC_START + ( staticPages.size() << 12 );
    if (vaddr < staticBoundary && vaddr >= STATIC_START) {
        size_t index = (vaddr - STATIC_START) >> 12;
        return staticPages[index];
    }

    size_t textBoundary = TEXT_START + ( textPages.size() << 12 );
    if (vaddr < textBoundary && vaddr >= TEXT_START) {
        size_t index = (vaddr - TEXT_START) >> 12;
        return textPages[index];
    }

    return ministl::nullopt;
}

bool kernel::SegmentedPageTable::mapPTE(const Entry& pte, uint32_t vpn) {
    size_t pageBoundary = vpn << 12;
    
    size_t stackBoundary = STACK_LIMIT - ( stackPages.size() << 12 );
    if (pageBoundary < STACK_LIMIT && pageBoundary >= stackBoundary) {
        // since the stack grows down, the calculation for index into the vector is (0x80000000 - (vaddr & 0xFFF) ) >> 12 == 0x80000 - (vaddr >> 12)
        size_t index = 0x80000 - vpn;
        MemoryManager::instance().freeFrame( stackPages[vpn].pfn << 12 );
        stackPages[index] = pte;
        return true;
    }

    // add a check for vpn(stackBoundary) + 1 to add to the vector

    size_t dynamicBoundary = DYNAMIC_START + ( dynamicPages.size() << 12 );
    if (pageBoundary < dynamicBoundary && pageBoundary >= DYNAMIC_START) {
        size_t index = (pageBoundary - DYNAMIC_START) >> 12;
        MemoryManager::instance().freeFrame( dynamicPages[vpn].pfn << 12 );
        dynamicPages[index] = pte;
        return true;
    }

    if ( ((pageBoundary - DYNAMIC_START) >> 12) == dynamicPages.size() ) dynamicPages.push_back(pte);

    size_t staticBoundary = STATIC_START + ( staticPages.size() << 12 );
    if (pageBoundary < staticBoundary && pageBoundary >= STATIC_START) {
        size_t index = (pageBoundary - STATIC_START) >> 12;
        MemoryManager::instance().freeFrame( staticPages[vpn].pfn << 12 );
        staticPages[index] = pte;
        return true;
    }

    size_t textBoundary = TEXT_START + ( textPages.size() << 12 );
    if (pageBoundary < textBoundary && pageBoundary >= TEXT_START) {
        size_t index = (pageBoundary - TEXT_START) >> 12;
        MemoryManager::instance().freeFrame( textPages[vpn].pfn << 12 );
        textPages[index] = pte;
        return true;
    }

    return false;
}

kernel::SegmentedPageTable::~SegmentedPageTable() {
    for (size_t i = 0; i < stackPages.size(); ++i)      MemoryManager::instance().freeFrame( stackPages[i].pfn );
    for (size_t i = 0; i < dynamicPages.size(); ++i)    MemoryManager::instance().freeFrame( dynamicPages[i].pfn );
    for (size_t i = 0; i < staticPages.size(); ++i)     MemoryManager::instance().freeFrame( staticPages[i].pfn );
    for (size_t i = 0; i < textPages.size(); ++i)       MemoryManager::instance().freeFrame( textPages[i].pfn );
}