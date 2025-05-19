#include "Hardware.h"
#include "BinaryUtils.h"
#include "IInstruction.h"
#include "RInstruction.h"
#include "SpecialInstruction.h"
#include <iostream>
#include <iomanip>

Hardware::Memory::Iterator::Iterator(const std::unordered_map<Word, char>::const_iterator& src) {
    it = new std::unordered_map<Word, char>::const_iterator(src);
}

Hardware::Memory::Iterator::~Iterator() {
    delete it;
}

const std::pair<const Word, char>& Hardware::Memory::Iterator::operator*() const {
    return *(*it);
}

const std::pair<const Word, char>* Hardware::Memory::Iterator::operator->() const {
    return (*it).operator->();
}

Hardware::Memory::Iterator& Hardware::Memory::Iterator::operator++() {
    ++(*it);
    return *this;
}

Hardware::Memory::Iterator Hardware::Memory::Iterator::operator++(int) {
    auto tmp = *this;
    ++*this;
    return tmp;
}

bool Hardware::Memory::Iterator::operator==(const Iterator& other) const {
    return (*it) == *other.it;
}

bool Hardware::Memory::Iterator::operator!=(const Iterator& other) const {
    return (*it) != *other.it;
}

Hardware::Memory::Iterator Hardware::Memory::begin() const {
    return Iterator(RAM.cbegin());
}

Hardware::Memory::Iterator Hardware::Memory::end() const {
    return Iterator(RAM.cend());
}

// we using big endian around here

Hardware::Memory::Memory() {}
Hardware::Memory::Memory(const boundRegisters& bounds) {
    memoryBounds = bounds;
}

Word Hardware::Memory::getWord(const Word& addr) {
    Byte word[4] = {
        (Byte)RAM.try_emplace(addr,     0).first->second,
        (Byte)RAM.try_emplace(addr + 1, 0).first->second,
        (Byte)RAM.try_emplace(addr + 2, 0).first->second,
        (Byte)RAM.try_emplace(addr + 3, 0).first->second
    };

    return Binary::loadBigEndian(word);
}

void Hardware::Memory::setWord(const Word& addr, const Word& word) {
    RAM[addr] =     (word >> 24);
    RAM[addr + 1] = (word >> 16) & 0xFF;
    RAM[addr + 2] = (word >> 8) & 0xFF;
    RAM[addr + 3] =  word & 0xFF;
}

Hardware::Machine::Machine() {
    for (int i = 0; i < 32; ++i) registerFile[i] = 0;
    programCounter = 0x00400024;
    registerFile[Binary::SP] = 0x7fffffff;
    registerFile[Binary::GP] = 0x10008000; 
    killed = false;
}

const Word& Hardware::Machine::readProgramCounter() const {
    return programCounter;
}

const int& Hardware::Machine::readRegister(const Byte& reg) const {
    return registerFile[reg];
}

const Hardware::Memory& Hardware::Machine::readMemory() const {
    return RAM;
}

void Hardware::Machine::loadInstructions(const std::vector<Word>& instructions) {
    // for right now, just load according to mips for no patricular reason
    // will figure out exact specifications later

    Word at = 0x00400024;
    for (const auto& instr : instructions) {
        RAM.setWord(at, instr);
        at += 4;
    }
    
    RAM.memoryBounds.textBound = at;
    RAM.memoryBounds.stackBound = 0x7fffe000;
    RAM.memoryBounds.dynamicBound = 0x70000000;
    RAM.memoryBounds.staticBound = 0; // tbd 
}

void Hardware::Machine::runInstruction() {
    // std::cout << "READING INSTUCTION: " << std::hex << RAM.getWord(programCounter) << std::endl;

    if (programCounter >= RAM.memoryBounds.textBound) {
        std::cout << "Reading past text memory. Killing process..." << std::endl;
        killed = true;
        return;
    }

    auto it = instructionCache.find(programCounter);
    if (it != instructionCache.end()) {
        it->second->run();
        programCounter += 4;
        return;
    }

    (
        instructionCache[programCounter] = instructionFactory( RAM.getWord(programCounter), programCounter, registerFile, RAM, killed )
    )->run(); // cool syntax

    programCounter += 4;
}

void Hardware::Machine::run() {
    while (!killed) runInstruction();
}

// TODO: Add better error handling

std::unique_ptr<Hardware::Instruction> Hardware::instructionFactory(const Word& binary_instruction, Word& programCounter, int* registerFile, Hardware::Memory& RAM, bool& kill_flag) {
    // DECODE
    using namespace Binary;
    
    Opcode opcode = Opcode((binary_instruction >> 26) & 0b111111);  // For All

    Word address = binary_instruction & 0x1FFFFFF;                  // For Jump

    Register rs = Register((binary_instruction >> 21) & 0b11111);   // For I/R
    Register rt = Register((binary_instruction >> 16) & 0b11111);   // For I/R


    Register rd = Register((binary_instruction >> 11) & 0b11111);   // For R
    Byte shamt = (binary_instruction >> 6) & 0b11111;               // For R
    Funct funct = Funct(binary_instruction & 0b111111);             // For R

    short immediate = binary_instruction & 0xFFFF;                  // For I

    // Return instruction

    #define R_VAR_INIT(x) std::make_unique<x>(registerFile[rd], registerFile[rt], registerFile[rs])
    #define R_SHFT_INIT(x) std::make_unique<x>(registerFile[rd], registerFile[rt], shamt)
    if (!opcode) {  // Is an R-Type Instruction
        switch (funct) {
            case ADD:
                return R_VAR_INIT(Add);
            case ADDU:
                return R_VAR_INIT(AddUnsigned);
            case AND:
                return R_VAR_INIT(And);
            case NOR:
                return R_VAR_INIT(Nor);
            case OR:
                return R_VAR_INIT(Or);
            case SLT:
                return R_VAR_INIT(SetLessThan);
            case SLTU:
                return R_VAR_INIT(SetLessThanUnsigned);
            case SLL:
                return R_SHFT_INIT(ShiftLeftLogical);
            case SRL:
                return R_SHFT_INIT(ShiftRightLogical);
            case SUB:
                return R_VAR_INIT(Subtract);
            case SUBU:
                return R_VAR_INIT(SubtractUnsigned);
            case JR:
                return std::make_unique<JumpRegister>(programCounter, registerFile[RA]);
            case SYSCALL:
                return std::make_unique<Syscall>(registerFile[V0], registerFile[A0], registerFile[A1], kill_flag);
            default:
                std::cout << "hello from bad funct " << std::hex << Word(binary_instruction) << std::endl;
                throw 1;
        }
    }

    // TODO: Figure out how address works for J/JAL
    // TODO: Make offset work for branch
    // TODO: More memory instr

    #define I_GEN_INIT(x) std::make_unique<x>(registerFile[rt], registerFile[rs], immediate)
    #define I_MEM_INIT(x) std::make_unique<x>(registerFile[rt], registerFile[rs], immediate, RAM)
    #define I_BRANCH_INIT(x) std::make_unique<x>(registerFile[rt], registerFile[rs], immediate, programCounter)
    switch (opcode) {
        case ADDI:
            return I_GEN_INIT(AddImmediate);
        case ADDIU:
            return I_GEN_INIT(AddImmediateUnsigned);
        case ANDI:
            return I_GEN_INIT(AndImmediate);
        case SLTI:
            return I_GEN_INIT(SetLessThanImmediate);
        case SLTIU:
            return I_GEN_INIT(SetLessThanImmediateUnsigned);
        case ORI:
            return I_GEN_INIT(OrImmediate);
        case LW:
            return I_MEM_INIT(LoadWord);
        case SW:
            return I_MEM_INIT(StoreWord);
        case BEQ:
            return I_BRANCH_INIT(BranchOnEqual);
        case BNE:
            return I_BRANCH_INIT(BranchOnNotEqual);
        case J:
            return std::make_unique<Jump>(programCounter, address);
        case JAL:
            return std::make_unique<JumpAndLink>(programCounter, address, registerFile[RA]);
        case LUI:
            return std::make_unique<LoadUpperImmediate>(registerFile[rt], immediate);
        default:
            std::cout << "hello from bad opcode " << std::hex << Word(binary_instruction) << std::endl;
            throw 2;
    }

    throw 3;
    return nullptr;
    
}