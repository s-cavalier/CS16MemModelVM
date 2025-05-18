#ifndef __BINARY_UTILS_H__
#define __BINARY_UTILS_H__

// TODO: ADD MORE INSTRUCTIONS
// Maybe put in namespace to clear up global namespace?

namespace Binary {
    enum Opcode : unsigned char {
        R_TYPE = 0x0,
        J = 0x2,
        JAL = 0x3,

        ADDI = 0x8,
        ADDIU = 0x9,
        SLTI = 0xa,
        SLTIU = 0xb,
        ANDI = 0xc,
        BEQ = 0x4,
        BNE = 0x5,
        LBU = 0x24,
        LHU = 0x25,
        LL = 0x30,
        LUI = 0xf,
        LW = 0x23,
        ORI = 0xd,
        SB = 0x28,
        SC = 0x38,
        SH = 0x29,
        SW = 0x2b
    };

    enum Funct : unsigned char {
        SYSCALL = 0xc,
        ADD = 0x20,
        ADDU = 0x21,
        AND = 0x24,
        JR = 0x8,
        NOR = 0x27,
        OR = 0x25,
        SLT = 0x2a,
        SLTU = 0x2b,
        SLL = 0x0,
        SRL = 0x2,
        SUB = 0x22,
        SUBU = 0x23
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

    inline unsigned int loadBigEndian(const unsigned char bytes[4]) {
        return (((unsigned int)(bytes[0])) << 24) |
            (((unsigned int)(bytes[1])) << 16) |
            (((unsigned int)(bytes[2])) << 8)  |
            (((unsigned int)(bytes[3]))); 
    }

    static char regToString[32][3] = {
        "0\0",  // null terminator so it ends earlier
        "at",
        "v0",
        "v1",
        "a0",
        "a1",
        "a2",
        "a3",
        "t0",
        "t1",
        "t2",
        "t3",
        "t4",
        "t5",
        "t6",
        "t7",
        "s0",
        "s1",
        "s2",
        "s3",
        "s4",
        "s5",
        "s6",
        "s7",
        "t8",
        "t9",
        "k0",
        "k1",
        "gp",
        "sp",
        "fp",
        "ra"
    };

}


#endif