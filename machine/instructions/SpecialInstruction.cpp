#include "SpecialInstruction.h"
#include "../BinaryUtils.h"
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

HiLoInstruction::HiLoInstruction(Hardware::HiLoRegisters& hiLo) : hiLo(hiLo) { }
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

Syscall::Syscall(Hardware::Machine& machine) : machine(machine) {}
void Syscall::run() {
    const auto& cpu = machine.readCPU();
    const auto& RAM = machine.readMemory();
    using namespace Binary;
    switch (cpu.readRegister(V0).ui) {
        case 1:         // Print integer
            std::cout << cpu.readRegister(A0).i;
            return;
        case 4:
            for (Word i = cpu.readRegister(A0).ui; RAM.getByte(i) != '\0'; ++i) std::cout << RAM.getByte(i);
            return; 
        case 10:        // exit
            machine.killed = true;
            return;
        case 11:
            std::cout << char(cpu.readRegister(A0).ui);
            return;
        default:
            std::cout << "hello from bad syscall " << cpu.readRegister(V0).ui << std::endl;
            throw 4;
    }
}