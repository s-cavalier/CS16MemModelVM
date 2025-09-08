#ifndef __ASM_INTERFACE_H__
#define __ASM_INTERFACE_H__

#define VMTUNNEL ".word 0x0000003F\n"

namespace kernel {
    using uint32_t = unsigned int;

    enum Register : unsigned char {
        ZERO = 0,
        AT = 1,
        V0 = 2,
        V1 = 3,
        A0 = 4,
        A1 = 5,
        A2 = 6,
        A3 = 7,
        T0 = 8,
        T1 = 9,
        T2 = 10,
        T3 = 11,
        T4 = 12,
        T5 = 13,
        T6 = 14,
        T7 = 15,
        S0 = 16,
        S1 = 17,
        S2 = 18,
        S3 = 19,
        S4 = 20,
        S5 = 21,
        S6 = 22,
        S7 = 23,
        T8 = 24,
        T9 = 25,
        K0 = 26,
        K1 = 27,
        GP = 28,
        SP = 29,
        FP = 30,
        RA = 31
    };


    struct RegisterContext {
        uint32_t regs[31]; // doesn't include $zero
        uint32_t hiRegister;
        uint32_t loRegister;
        uint32_t epc;
        uint32_t status;
        uint32_t cause;
    
        uint32_t& accessRegister(Register reg) { return regs[reg - 1]; }

        RegisterContext();
        RegisterContext(const RegisterContext& other);              // copy and assignment to manage the regs ptr properly
        RegisterContext& operator=(const RegisterContext& other);

    };
    
    enum VMRequestType : uint32_t {
        UNKNOWN,
        HALT,               
        PRINT_STRING,     
        PRINT_INTEGER,
        READ_INTEGER,

        FOPEN,
        FREAD,
        FWRITE,
        FSEEK,
        FCLOSE
    };

    enum FileFlags {
        O_RDONLY = 0,
        O_WRONLY = 1,
        O_RDWR   = 2,
        O_CREAT  = 0x40,
        O_EXCL   = 0x80,
        O_TRUNC  = 0x200,
        O_APPEND = 0x400
    };

    struct VMResponse {
        uint32_t res;
        uint32_t err;
    };

    struct VMPackage {
        VMRequestType reqType;
        union {
            struct { uint32_t arg0; uint32_t arg1; uint32_t arg2; } generic;
            struct { uint32_t c_str; } printString;
            struct { uint32_t num; } printInteger;
            struct { uint32_t path; uint32_t flags; } fopen;
            struct { uint32_t fd; uint32_t buffer; uint32_t nbytes; } fread;
            struct { uint32_t fd; uint32_t buffer; uint32_t nbytes; } fwrite;
            struct { uint32_t fd; uint32_t offset; uint32_t whence; } fseek;
            struct { uint32_t fd; } fclose;
        } args;


        // No-args request
        VMPackage(VMRequestType reqType) : reqType(reqType) {}

        // Argument request
        VMPackage(VMRequestType reqType, uint32_t arg0, uint32_t arg1 = 0, uint32_t arg2 = 0) : reqType(reqType), args{arg0, arg1, arg2} {}

        // File-based request
        VMPackage(const char* filePath, uint32_t fileFlags, VMRequestType openType);

        VMResponse send() const;

    };

    int getK0Register();
    int getK1Register();
    uint32_t getBadVAddr();

}

#define Halt                    kernel::VMPackage(kernel::HALT).send()
#define PrintString(ptr)        kernel::VMPackage(kernel::PRINT_STRING, (kernel::uint32_t)(ptr) ).send()
#define PrintWrapped(l, m, r)   PrintString(l); PrintInteger(m); PrintString(r)
#define PrintInteger(num)       kernel::VMPackage(kernel::PRINT_INTEGER, (num)).send()
#define ReadInteger             kernel::VMPackage(kernel::READ_INTEGER).send()

#endif