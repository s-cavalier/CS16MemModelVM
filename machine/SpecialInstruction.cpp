#include "SpecialInstruction.h"
#include "BinaryUtils.h"
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

Syscall::Syscall(Hardware::Machine::RegisterFile& rf, bool& kill_flag) : rf(rf), kill_flag(kill_flag) {}
void Syscall::run() {
    using namespace Binary;
    switch (rf.registerFile[V0]) {
        case 1:         // Print integer
            std::cout << rf.registerFile[A0];
            return;
        case 2:
            std::cout << rf.fpRegisterFile[12];
            return;
        case 4:
            for (Word i = rf.registerFile[A0]; rf.RAM.getByte(i) != '\0'; ++i) std::cout << rf.RAM.getByte(i);
            return; 
        case 10:        // exit
            kill_flag = true;
            return;
        case 11:
            std::cout << char(rf.registerFile[A0]);
            return;
        default:
            std::cout << "hello from bad syscall " << rf.registerFile[V0] << std::endl;
            throw 4;
    }
}