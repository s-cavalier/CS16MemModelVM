#include "ASMInterface.h"

kernel::VMPackage::VMPackage(const char* filePath, uint32_t fileFlags, VMRequestType openType) {
    reqType = FOPEN;
    args.fopen.path = (uint32_t)filePath;
    args.fopen.flags = fileFlags;
    VMResponse fd = send();
    reqType = openType;
    args.generic = { fd.res, fd.err, 0 };
}

kernel::VMResponse kernel::VMPackage::send() const {
    if (reqType == UNKNOWN) return {0, 0};

    VMResponse res;

    __asm__ volatile (
        "move $a0, %0\n\t"            // Place &Request
        "move $v0, %1\n\t"            // Place &Response
        VMTUNNEL
        :
        : "r"(this), "r"(&res)
        : "$a0", "$v0", "memory"
    );

    return res;
}

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
