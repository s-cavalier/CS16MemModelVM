#include <iostream>
#include "HeapManager.h"

// A class to test construction/destruction
struct Foo {
    int x;
    Foo(int val) : x(val) {}
};

#define PrintString(x) std::cout << (x)
#define PrintInteger(x) PrintString(x)

void* operator new(size_t size) { return Heap::freeList.malloc(size); }
void* operator new[](size_t size) { return Heap::freeList.malloc(size); }
void operator delete(void* ptr) noexcept {return Heap::freeList.free(ptr); }
void operator delete[](void* ptr) noexcept {  return Heap::freeList.free(ptr); }
void operator delete(void* ptr, size_t) noexcept {  return Heap::freeList.free(ptr); }
void operator delete[](void* ptr, size_t) noexcept { return Heap::freeList.free(ptr); }

struct Block {
    char data[32];

    Block(char fill) {
        for (int i = 0; i < 32; ++i) data[i] = fill;
        PrintString("Constructed block with fill: ");
        PrintInteger((int)fill);
        PrintString("\n");
    }

    ~Block() {
        PrintString("Destructed block\n");
    }

    bool check(char expected) {
        for (int i = 0; i < 32; ++i)
            if (data[i] != expected) return false;
        return true;
    }
};

void HeapTest() {
    PrintString("=== HEAP TEST START ===\n");

    // Allocate two blocks
    Block* a = new Block('A');
    Block* b = new Block('B');

    // Free the first, then reallocate a smaller one
    delete a;
    Block* c = new Block('C');

    // Check if c reused a’s memory (optional)
    if ((void*)a == (void*)c) {
        PrintString("Reused freed block\n");
    } else {
        PrintString("Did not reuse freed block\n");
    }

    // Check that block b is intact
    if (b->check('B')) {
        PrintString("Block B intact\n");
    } else {
        PrintString("Block B corrupted\n");
    }

    // Delete remaining blocks
    delete b;
    delete c;

    // Test allocation alignment
    Block* aligned = new Block('D');
    if (((uintptr_t)aligned & 0x3) == 0) {
        PrintString("Alignment OK\n");
    } else {
        PrintString("Alignment WRONG\n");
    }
    delete aligned;

    PrintString("=== HEAP TEST END ===\n");
}

int main() {
    std::cout << sizeof(Heap::BlockHeader) << std::endl << std::endl;

    HeapTest();

    return 0;
}