#include "Memory.h"
#include "BinaryUtils.h"

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

Word Hardware::Memory::getWord(const Word& addr) const {
    Byte word[4] = {
        getByte(addr),
        getByte(addr + 1),
        getByte(addr + 2),
        getByte(addr + 3)
    };

    return Binary::loadBigEndian(word);
}

HalfWord Hardware::Memory::getHalfWord(const Word& addr) const {
    Byte halfword[2] = {
        getByte(addr),
        getByte(addr + 1),
    };

    return (halfword[0] << 8) | halfword[1];
}

Byte Hardware::Memory::getByte(const Word& addr) const {
    auto ret = RAM.find(addr);
    return ret != RAM.end() ? ret->second : 0;
}

void Hardware::Memory::setWord(const Word& addr, const Word& word) {
    RAM[addr] =     (word >> 24);
    RAM[addr + 1] = (word >> 16) & 0xFF;
    RAM[addr + 2] = (word >> 8) & 0xFF;
    RAM[addr + 3] =  word & 0xFF;
}

void Hardware::Memory::setHalfWord(const Word& addr, const HalfWord& halfword) {
    RAM[addr]     = (halfword >> 8);
    RAM[addr + 1] =  halfword & 0xFF;
}

void Hardware::Memory::setByte(const Word& addr, const Byte& byte) {
    RAM[addr] = byte;
}