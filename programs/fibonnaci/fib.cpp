#include "../ministl/stdio.h"

int main() {

    std::printString("Enter a fibonnaci value:\n");
    int n = std::readInteger();

    __asm__ volatile (".word 0x0000003F");

    int a = 0, b = 1;
    for (int i = 2; i <= n; ++i) {
        int temp = b;
        b = a + b;
        a = temp;
    }

    std::printInteger(b);
    std::printString("\n");

    std::exit();
}