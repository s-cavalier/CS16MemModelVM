#include "SpecialInstruction.h"
#include "../BinaryUtils.h"

JInstruction::JInstruction(J_INSTR_ARGS) : pc(pc) {}

Jump::Jump(J_INSTR_ARGS, const Word& target) : JInstruction(pc) {
    this->target = (((pc + 4) & 0xF0000000) | (target << 2)) - 4;
}
void Jump::run() { pc = target; }

JumpAndLink::JumpAndLink(J_INSTR_ARGS, const Word& target, int& ra) : JInstruction(pc), ra(ra) {
    this->target = (((pc + 4) & 0xF0000000) | (target << 2)) - 4;
}
void JumpAndLink::run() { ra = pc + 4; pc = target; }

JumpRegister::JumpRegister(J_INSTR_ARGS, const int& ra) : JInstruction(pc), ra(ra) {}
void JumpRegister::run() { pc = ra - 4; }

JumpAndLinkRegister::JumpAndLinkRegister(J_INSTR_ARGS, int& rd, const int& rs) : JInstruction(pc), rd(rd), rs(rs) {}
void JumpAndLinkRegister::run() { rd = pc + 4; pc = rs - 4; }

HiLoInstruction::HiLoInstruction(Hardware::HiLoRegisters& hiLo) : hiLo(hiLo) { }
HLMoveInstruction::HLMoveInstruction(HL_MOVE_INSTR_ARGS) : HiLoInstruction(hiLo), storage_register(storage_register) {}
HLOpInstruction::HLOpInstruction(HL_OP_INSTR_ARGS) : HiLoInstruction(hiLo), rs(rs), rt(rt) {}

#define HL_MOVE_CNSTCTR_INIT(arg) arg::arg(HL_MOVE_INSTR_ARGS) : HLMoveInstruction(storage_register, hiLo) {} void arg::run()
HL_MOVE_CNSTCTR_INIT(MoveFromHi) { storage_register = hiLo.hi; }
HL_MOVE_CNSTCTR_INIT(MoveFromLo) { storage_register = hiLo.lo; }
HL_MOVE_CNSTCTR_INIT(MoveToHi) { hiLo.hi = storage_register; }
HL_MOVE_CNSTCTR_INIT(MoveToLo) { hiLo.lo = storage_register; }

#define HL_OP_CNSTCTR_INIT(arg) arg::arg(HL_OP_INSTR_ARGS) : HLOpInstruction(rs, rt, hiLo) {} void arg::run()
HL_OP_CNSTCTR_INIT(Multiply) { DoubleWord res = (long long)(rs) * (long long)(rt); hiLo.lo = res; hiLo.hi = res >> 32; }
HL_OP_CNSTCTR_INIT(MultiplyUnsigned) { DoubleWord res = DoubleWord(Word(rs)) * DoubleWord(Word(rt)); hiLo.lo = Word(res); hiLo.hi = Word(res >> 32); }
HL_OP_CNSTCTR_INIT(Divide) { hiLo.hi = rs % rt; hiLo.lo = rs / rt; }
HL_OP_CNSTCTR_INIT(DivideUnsigned) { hiLo.hi = Word(rs) % Word(rt); hiLo.lo = Word(rs) / Word(rt); }

TrapIfEqual::TrapIfEqual(const int& rs, const int& rt) : rs(rs), rt(rt) {}
void TrapIfEqual::run() { if (rs == rt) throw Hardware::Trap(Hardware::Trap::TRAP); }

Syscall::Syscall() {}
void Syscall::run() { throw Hardware::Trap(Hardware::Trap::SYSCALL); }