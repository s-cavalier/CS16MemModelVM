#ifndef __HEAP_MANAGER_H__
#define __HEAP_MANAGER_H__

// remove if need be
#define TESTING

#ifdef TESTING
    #include <unistd.h>
#endif

// TODO: ADD IN CHECKS FOR:
// Double frees, free(0), access after free

#define align4(x) (((x) + 3) & ~0x3)

typedef decltype(sizeof(0)) size_t;

namespace Heap {

    struct BlockHeader {
        size_t size;
        BlockHeader* next;
        BlockHeader* prev;
        int free;   // bool
        void* ptr;
        char data[1];
    };

    #define BLOCKHEADER_OFFSET ((size_t)&(((Heap::BlockHeader*)0)->data))

    // Padding will force the data field to add to the size in full

    class FreeList {
        size_t block_count;
        size_t total_size;
        BlockHeader* head;
        BlockHeader* last_visited;

        BlockHeader* getBlock(void* p);
        BlockHeader* findBlock(size_t s);
        BlockHeader* extendHeap(size_t s);
        void splitBlock(BlockHeader* block, size_t s);
        bool validAddr(void* p);
        BlockHeader* fusion(BlockHeader* block);

    public:
        FreeList() : block_count(0), total_size(0), head(nullptr), last_visited(nullptr) {}

        inline const size_t& totalBytes() const { return total_size; }
        inline const size_t& totalBlocks() const { return block_count; }

        void* malloc(size_t s);
        void* calloc(size_t number, size_t size);
        void free(void* ptr);
    };

    static FreeList freeList;

}

#endif