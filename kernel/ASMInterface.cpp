#include "ASMInterface.h"

kernel::RegisterContext::RegisterContext() : regs{0}, epc(0), status(0), cause(0) {}
kernel::RegisterContext::RegisterContext(const RegisterContext& other) : epc(other.epc), status(other.status), cause(other.cause) {
    for (unsigned char i = 0; i < 31; ++i) regs[i] = other.regs[i];
}

kernel::RegisterContext& kernel::RegisterContext::operator=(const RegisterContext& other) {
    if (this == &other) return *this;
    epc = other.epc;
    status = other.status;
    cause = other.cause;

    for (unsigned char i = 0; i < 31; ++i) regs[i] = other.regs[i];
    
    return *this;
}

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

kernel::uint32_t kernel::getBadVAddr() {
    uint32_t value;
    __asm__ volatile (
        "mfc0 %0, $8\n"
        : "=r"(value)
        :
        : "memory"
    );
    return value;
}