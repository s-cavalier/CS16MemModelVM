#include "../ministl/asmio.h"

int main() {

    unsigned int pid = std::fork();

    if (pid) {
        std::printString("Hello from the parent! My kid's pid is: ");
        std::printString("\n");
        std::printInteger(pid);
        std::printString("Anyways, I'm done.\n");
    } else {
        std::printString("Hello, I'm the child! Gonna die now.\n");
    }

    std::exit();
    return 0;
}