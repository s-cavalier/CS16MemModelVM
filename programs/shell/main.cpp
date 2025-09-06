#include "../ministl/asmio.h"
#include "../ministl/Vector.h"

struct Foo {
    virtual void foo() { std::printString("Foo::foo()\n"); }
    virtual ~Foo() = default;
};

struct Bar : public Foo {
    virtual void foo() { std::printString("Bar::foo()\n"); }
};

int main() {

    std::vector< std::unique_ptr<Foo> > dynamicDispatchArr(5);

    for (size_t i = 0; i < dynamicDispatchArr.size(); ++i) dynamicDispatchArr[i] = std::make_unique<Bar>();
    for (size_t i = 0; i < dynamicDispatchArr.size(); ++i) dynamicDispatchArr[i]->foo();

    std::exit();
    return 0;
}