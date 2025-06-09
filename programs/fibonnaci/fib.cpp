#include "../ministl/stdio.h"

int fib(int n) {
    if (n < 0) return -1;
    if (n == 0) return 0;

    int a = 0, b = 1;
    for (int i = 2; i <= n; ++i) {
        int temp = b;
        b = a + b;
        a = temp;
    }

    return b;
}

int main() {

    std::printString("Enter a fibonnaci value:\n");
    int n = std::readInteger();

    int out = fib(n);

    std::printInteger(out);
    std::printString("\n");

    std::exit();
}