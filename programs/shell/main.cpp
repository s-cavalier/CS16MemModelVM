#include "../ministl/asmio.h"
#include "../ministl/Vector.h"

struct Foo {

    Foo() { std::printString("fooconstr\n"); }
    virtual void bar() { std::printString("foo::bar\n"); }

};

struct Bar : public Foo {

    Bar() { std::printString("Barconstrt\n"); }
    virtual void bar() { std::printString("lgob\n"); }

};

int main() {

    std::unique_ptr<Foo> x[] = { std::make_unique<Bar>(), std::make_unique<Bar>(), std::make_unique<Bar>(), std::make_unique<Bar>() };

    for (int i = 0; i < 4; ++i) x[i]->bar();

    std::printString("now forking...\n");

    unsigned int pid = std::fork();

    if (pid) {
        std::printString("i am parent\n");
    } else {
        std::printString("i am child\n");
    }


    return 0;
}