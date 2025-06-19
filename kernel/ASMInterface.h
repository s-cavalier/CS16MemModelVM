#ifndef __ASM_INTERFACE_H__
#define __ASM_INTERFACE_H__

#define VMTUNNEL ".word 0x0000003F\n"

namespace kernel {
    using uint32_t = unsigned int;

    struct TrapFrame {
        unsigned int at = 0;
        unsigned int v0 = 0;
        unsigned int v1 = 0;
        unsigned int a0 = 0;
        unsigned int a1 = 0;
        unsigned int a2 = 0;
        unsigned int a3 = 0;

        unsigned int t0 = 0;
        unsigned int t1 = 0;
        unsigned int t2 = 0; 
        unsigned int t3 = 0; 
        unsigned int t4 = 0; 
        unsigned int t5 = 0; 
        unsigned int t6 = 0; 
        unsigned int t7 = 0; 

        unsigned int s0 = 0; 
        unsigned int s1 = 0; 
        unsigned int s2 = 0; 
        unsigned int s3 = 0; 
        unsigned int s4 = 0; 
        unsigned int s5 = 0; 
        unsigned int s6 = 0; 
        unsigned int s7 = 0; 

        unsigned int t8 = 0; 
        unsigned int t9 = 0; 

        unsigned int k0 = 0;    
        unsigned int k1 = 0;    
        unsigned int gp = 0; 
        unsigned int sp = 0;    
        unsigned int fp = 0; 
        unsigned int ra = 0; 

        unsigned int epc = 0;
        unsigned int status = 0;
        unsigned int cause = 0;

    };

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

    // frame needs to be manually loaded
    TrapFrame* loadTrapFrame();

    int getK0Register();
    int getK1Register();

}

#define Halt                    kernel::VMPackage(kernel::HALT).send()
#define PrintString(ptr)        kernel::VMPackage(kernel::PRINT_STRING, (kernel::uint32_t)(ptr) ).send()
#define PrintWrapped(l, m, r)   PrintString(l); PrintInteger(m); PrintString(r)
#define PrintInteger(num)       kernel::VMPackage(kernel::PRINT_INTEGER, (num)).send()
#define ReadInteger             kernel::VMPackage(kernel::READ_INTEGER).send()

#endif