#include "../ministl/asmio.h"
#include "../ministl/Vector.h"

struct Foo {

    Foo() { std::printString("Penisfart\n"); }
    virtual void bar() { std::printString("poopbut\n"); }

};

struct Bar : public Foo {

    Bar() { std::printString("Barbut\n"); }
    virtual void bar() { std::printString("lgob\n"); }

};

int main() {

    std::unique_ptr<Foo> x[] = { std::make_unique<Bar>(), std::make_unique<Bar>(), std::make_unique<Bar>(), std::make_unique<Bar>() };

    for (int i = 0; i < 4; ++i) x[i]->bar();

    return 0;
}