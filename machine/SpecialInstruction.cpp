#include "SpecialInstruction.h"
#include <iostream>
#include <cassert>

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

HiLoInstruction::HiLoInstruction(Hardware::Machine::HiLoRegisters& hiLo) : hiLo(hiLo) { }
HLMoveInstruction::HLMoveInstruction(HL_MOVE_INSTR_ARGS) : HiLoInstruction(hiLo), storage_register(storage_register) {}
HLOpInstruction::HLOpInstruction(HL_OP_INSTR_ARGS) : HiLoInstruction(hiLo), rs(rs), rt(rt) {}

#define HL_MOVE_CNSTCTR_INIT(arg) arg::arg(HL_MOVE_INSTR_ARGS) : HLMoveInstruction(storage_register, hiLo) {} void arg::run()
HL_MOVE_CNSTCTR_INIT(MoveFromHi) { storage_register = hiLo.hi; }
HL_MOVE_CNSTCTR_INIT(MoveFromLo) { storage_register = hiLo.lo; }
HL_MOVE_CNSTCTR_INIT(MoveToHi) { hiLo.hi = storage_register; }
HL_MOVE_CNSTCTR_INIT(MoveToLo) { hiLo.lo = storage_register; }

#define HL_OP_CNSTCTR_INIT(arg) arg::arg(HL_OP_INSTR_ARGS) : HLOpInstruction(rs, rt, hiLo) {} void arg::run()
HL_OP_CNSTCTR_INIT(Multiply) { DoubleWord res = rs * rt; hiLo.lo = Word(res); res >>= 32; hiLo.hi = Word(res); }
HL_OP_CNSTCTR_INIT(MultiplyUnsigned) { DoubleWord res = Word(rs) * Word(rt); hiLo.lo = Word(res); res >>= 32; hiLo.hi = Word(res); }
HL_OP_CNSTCTR_INIT(Divide) { hiLo.hi = rs % rt; hiLo.lo = rs / rt; }
HL_OP_CNSTCTR_INIT(DivideUnsigned) { hiLo.hi = Word(rs) % Word(rt); hiLo.lo = Word(rs) / Word(rt); }

Syscall::Syscall(int& v0, int& a0, int& a1, float& f12, bool& kill_flag) : v0(v0), a0(a0), a1(a1), f12(f12), kill_flag(kill_flag) {}
void Syscall::run() {
    switch (v0) {
        case 1:         // Print integer
            std::cout << a0;
            return;
        case 2:
            std::cout << f12;
            return;
        case 10:        // exit
            kill_flag = true;
            return;
        case 11:
            std::cout << char(a0);
            return;
        default:
            std::cout << "hello from bad syscall " << v0 << std::endl;
            throw 4;
    }
}