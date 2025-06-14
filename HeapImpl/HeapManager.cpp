#include "HeapManager.h"

Heap::BlockHeader* Heap::FreeList::getBlock(void* p) {
    return (BlockHeader*)((char*)p - BLOCKHEADER_OFFSET);

}

bool Heap::FreeList::validAddr(void* p) {
    return (head) && ( p > head ) && ( p < CURR_BRK ) && ( p == getBlock(p)->ptr );
}

Heap::BlockHeader* Heap::FreeList::findBlock(size_t s) {
    BlockHeader* it = head;
    while (it && !(it->free && it->size >= s)) {
        last_visited = it;
        it = it->next; 
    }
    return it;
}

Heap::BlockHeader* Heap::FreeList::extendHeap(size_t s) {
    BlockHeader* it = (BlockHeader*)CURR_BRK;
    long long sb = (long long)sbrk(BLOCKHEADER_OFFSET + s);
    if (sb < 0) return nullptr;
    it->size = s;
    it->next = nullptr;
    it->prev = last_visited;
    it->ptr = it->data;
    if (last_visited) last_visited->next = it;
    it->free = 0;
    return it;
}

void Heap::FreeList::splitBlock(BlockHeader* block, size_t s) {
    BlockHeader* add = (BlockHeader*)(block->data + s);
    add->size = block->size - s - BLOCKHEADER_OFFSET;
    add->next = block->next;
    add->prev = block;
    add->free = 1;
    block->size = s;
    block->next = add;
    if (add->next) add->next->prev = add;
}

void* Heap::FreeList::malloc(size_t size) {
    BlockHeader* it;
    size_t s = align4(size);
    if (head) {
        last_visited = head;
        it = findBlock(s);
        if (it) {
            if ((it->size - s) >= (BLOCKHEADER_OFFSET + 4)) splitBlock(it, s);
            it->free = 0;
        } else {
            it = extendHeap(s);
            if (!it) return nullptr;
        }
    } else {
        it = extendHeap(s);
        if (!it) return nullptr;
        head = it;
    }
    return (it->data);
}

void* Heap::FreeList::calloc(size_t number, size_t size) {
    size_t total = number * size;
    void* ptr = malloc(total);
    if (ptr) {
        char* p = (char*)ptr;
        for (size_t i = 0; i < total; ++i) {
            p[i] = 0;
        }
    }
    return ptr;
}


Heap::BlockHeader* Heap::FreeList::fusion(BlockHeader* block) {
    if (block->next && block->next->free) {
        block->size += BLOCKHEADER_OFFSET + block->next->size;
        block->next = block->next->next;
        if (block->next) block->next->prev = block;
    }
    return block;
}

void Heap::FreeList::free(void* ptr) {
    BlockHeader* it;
    if (!validAddr(ptr)) return;
    it = getBlock(ptr);
    it->free = 1;
    if (it->prev && it->prev->free) it = fusion(it->prev);
    if (it->next) fusion(it);
    else {
        if (it->prev) it->prev->next = nullptr;
        else head = nullptr;
        SET_BRK(it);
    }
}

void* operator new(size_t size) { return Heap::freeList.malloc(size); }
void* operator new[](size_t size) { return Heap::freeList.malloc(size); }
void operator delete(void* ptr) noexcept { return Heap::freeList.free(ptr); }
void operator delete[](void* ptr) noexcept { return Heap::freeList.free(ptr); }