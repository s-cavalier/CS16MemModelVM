#ifndef __HEAP_MANAGER_H__
#define __HEAP_MANAGER_H__

typedef decltype(sizeof(0)) size_t;

void* kmalloc(size_t bytes);
void kfree(void* ptr);

void* operator new(size_t size) { return kmalloc(size); }
void operator delete(void* ptr) noexcept { return kfree(ptr); }

#endif