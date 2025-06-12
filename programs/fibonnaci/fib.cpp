#include "../ministl/stdio.h"

struct A {
    A() { std::printString("Hello from A!\n"); };

    virtual void foo() { std::printString("Dynamic dispatch from A!\n"); }
};

struct B : A {
    B() { std::printString("Hello from B!\n"); }

    virtual void foo() override { std::printString("Dynamic dispatch from B!\n"); }
};

int main() {
    
    B b;
    A* a = &b;

    a->foo();

    std::exit();
    return 0;
}