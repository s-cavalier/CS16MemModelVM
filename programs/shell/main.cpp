#include "../ministl/asmio.h"

int main() {

    unsigned int pid = std::fork();

    if (pid) {
        std::printString("Hello from the parent process!\n");
    } else {
        std::exec("programs/calculator/calculator.elf");
    }



    std::exit();
    return 0;
}