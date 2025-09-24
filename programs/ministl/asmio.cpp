#include "asmio.h"

void std::exit() {
    __asm__ volatile (
        "li   $v0, 10\n" 
        "syscall\n"
        :                 
        :                 
        : "$v0", "memory" 
    );

}


void std::printInteger(const int& i) {
    __asm__ volatile (
        "move  $a0, %0\n"
        "li    $v0, 1\n"
        "syscall\n"
        :
        : "r"(i)                
        : "$a0", "$v0", "memory"
    );
}

int std::readInteger() {
    int value;
    __asm__ volatile (
        "li   $v0, 5\n"
        "syscall\n"
        "move %0, $v0\n"
        : "=r"(value)        
        :                    
        : "$v0", "memory"
    );
    return value;
}

void std::printString(const char* str) {
    __asm__ volatile (
        "move  $a0, %0\n"
        "li    $v0, 4\n"
        "syscall\n"
        :
        : "r"(str)                
        : "$a0", "$v0", "memory"
    );
};

size_t std::readString(char* buf, size_t numbytes) {
    size_t bytes;
    __asm__ volatile (
        "li   $v0, 6\n"
        "move   $a0, %1\n"
        "move   $a1, %2\n"
        "syscall\n"
        "move %0, $v0\n"
        : "=r"(bytes)        
        : "r"(buf), "r"(numbytes)                    
        : "$v0", "$a0", "$a1", "memory"
    );
    return bytes;

}

unsigned int std::fork() {
    unsigned int value;
    __asm__ volatile (
        "li $v0, 57\n"
        "syscall\n"
        "move %0, $v0\n"
        : "=r"(value)
        :
        : "$v0", "memory"
    );

    return value;
}

void std::exec(const char* file) {
    __asm__ volatile (
        "move $a0, %0\n"
        "li $v0, 11\n"
        "syscall\n"
        : 
        : "r"(file)
        : "$a0", "$v0", "memory"
    );
}