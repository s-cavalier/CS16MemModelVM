#include "SpecialInstruction.h"
#include <iostream>

JInstruction::JInstruction(J_INSTR_ARGS) : pc(pc) {}

Jump::Jump(J_INSTR_ARGS, const Word& target) : JInstruction(pc) {
    this->target = (((pc + 4) & 0xF0000000) | (target << 2)) - 4;
}
void Jump::run() { pc = target; }

JumpAndLink::JumpAndLink(J_INSTR_ARGS, const Word& target, int& ra) : JInstruction(pc), ra(ra) {
    this->target = (((pc + 4) & 0xF0000000) | (target << 2)) - 4;
}
void JumpAndLink::run() { ra = pc; pc = target; }

JumpRegister::JumpRegister(J_INSTR_ARGS, const int& ra) : JInstruction(pc), ra(ra) {}
void JumpRegister::run() { pc = ra; }

JumpAndLinkRegister::JumpAndLinkRegister(J_INSTR_ARGS, int& rd, const int& rs) : JInstruction(pc), rd(rd), rs(rs) {}
void JumpAndLinkRegister::run() { rd = pc; pc = rs; }

HiLoInstruction::HiLoInstruction(long* hi_lo) : hi_lo(hi_lo) {}

Syscall::Syscall(int& v0, int& a0, int& a1, bool& kill_flag) : v0(v0), a0(a0), a1(a1), kill_flag(kill_flag) {}
void Syscall::run() {
    switch (v0) {
        case 1:         // Print integer
            std::cout << a0;
            return;
        case 10:        // exit
            kill_flag = true;
            return;
        default:
            std::cout << "hello from bad syscall " << v0 << std::endl;
            throw 4;
    }
}