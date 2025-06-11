#include "../ministl/stdio.h"

struct A {
    A() { std::printString("Hello from A!\n"); };

    virtual void foo() = 0;
};

struct B : A {
    B() { std::printString("Hello from B!\n"); }

    virtual void foo() { std::printString("Dynamic dispatch from B!"); }
};

int main() {
    
    B b;
    A* a = &b;

    a->foo();

    std::exit();
    return 0;
}