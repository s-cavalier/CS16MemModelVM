#include "ASMInterface.h"

kernel::TrapFrame* kernel::loadTrapFrame() {
    TrapFrame* ptr = (TrapFrame*)( (unsigned int)( kernel::getK0Register() ) );
    return ptr;
}

int kernel::getK0Register() {
    int value;
    __asm__ volatile (
        "move %0, $k0"
        : "=r"(value)
        :                
        :                
    );
    return value;
}

int kernel::getK1Register() {
    int value;
    __asm__ volatile (
        "move %0, $k1"
        : "=r"(value)
        :                
        :                
    );
    return value;
}

// moves cause into k0
void kernel::moveExceptionCode() {
    __asm__ volatile (
        "mfc0 $k0, $13\n"
        :
        :
        : "$k0"
    );
}


void kernel::halt() {
    __asm__ volatile (HALT);
}

void kernel::printInteger(const int& i) {
    __asm__ volatile (
        "move  $a0, %0\n"
        PRINTI
        :
        : "r"(i)                
        : "$a0", "memory"
    );
}

int kernel::readInteger() {
    int value;
    __asm__ volatile (
        READI
        "move %0, $v0\n"
        : "=r"(value)        
        :                    
        : "$v0", "memory"
    );
    return value;
}

void kernel::printString(const unsigned int& addr) {
    __asm__ volatile (
        "move $a0, %0\n"
        PRINTSTR
        :
        : "r"(addr)
        : "$a0", "memory"
    );
};
