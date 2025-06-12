#ifndef __HEAP_MANAGER_H__
#define __HEAP_MANAGER_H__

#define KERNEL_HEAP_START 0xC0000000


typedef decltype(sizeof(0)) size_t;

namespace Heap {

    struct BlockHeader {
        size_t block_size;
        BlockHeader* next;
        BlockHeader* prev;
    };

    static size_t constexpr BLOCKHEADER_OFFSET = sizeof(BlockHeader);

    class FreeList {
        size_t block_count;
        size_t total_size;
        BlockHeader* head;
        BlockHeader* tail;        

    public:

        inline BlockHeader* getHead() { return head; }
        inline const BlockHeader* getHead() const { return head; }
        inline BlockHeader* getTail() { return tail; }
        inline const BlockHeader* getTail() const { return tail; }
        inline const size_t& totalBytes() const { return total_size; }
        inline const size_t& totalBlocks() const { return block_count; }

        void* getMemory(size_t bytes);
    };


    void* heap_ptr = (void*)(KERNEL_HEAP_START + BLOCKHEADER_OFFSET);

    void* kmalloc(size_t bytes);
    void kfree(void* ptr);

}

void* operator new(size_t size) { return Heap::kmalloc(size); }
void operator delete(void* ptr) noexcept { return Heap::kfree(ptr); }

#endif