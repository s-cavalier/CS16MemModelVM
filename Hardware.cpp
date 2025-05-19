#include "Hardware.h"
#include "Instruction.h"
#include "BinaryUtils.h"
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