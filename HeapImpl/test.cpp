#include <iostream>
#include "HeapManager.h"

// A class to test construction/destruction
struct Test {
    int value;
    Test(int v = 0) : value(v) {
        std::cout << "Test(" << value << ") constructed\n";
    }
    ~Test() {
        std::cout << "Test(" << value << ") destroyed\n";
    }
};

int main() {
    std::cout << "\n--- Testing new/delete ---\n";
    Test* t = new Test(42);
    delete t;

    std::cout << "\n--- Testing new[]/delete[] ---\n";
    Test* arr = new Test[3];
    delete[] arr;

    return 0;
}