#ifndef __MEMORY_H__
#define __MEMORY_H__
#include <unordered_map>

using Byte = unsigned char;
using HalfWord = unsigned short;
using Word = unsigned int;

namespace Hardware {

    class Memory {
        std::unordered_map<Word, char> RAM;
    
    public:
        class Iterator {
            std::unordered_map<Word, char>::const_iterator it;

        public:
            using value_type = const std::pair<const Word, char>;
            using reference = value_type&;
            using pointer = value_type*;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::forward_iterator_tag;

            Iterator(const std::unordered_map<Word, char>::const_iterator& src);
            ~Iterator();

            reference operator*() const;
            pointer   operator->() const;

            Iterator& operator++();
            Iterator operator++(int);

            bool operator==(const Iterator& other) const;
            bool operator!=(const Iterator& other) const;
        };

        struct boundRegisters {
            Word textBound;
            Word staticBound;
            Word dynamicBound;
            Word stackBound;
        };

        boundRegisters memoryBounds;

        Iterator begin() const;
        Iterator end() const;

        Memory();
        Memory(const boundRegisters& bounds);

        Word getWord(const Word& addr) const;
        HalfWord getHalfWord(const Word& addr) const;
        Byte getByte(const Word& addr) const;

        void setWord(const Word& addr, const Word& word);
        void setHalfWord(const Word& addr, const HalfWord& halfword);
        void setByte(const Word& addr, const Byte& byte);

        float getSingle(const Word& addr) const;
        double getDouble(const Word& addr) const;

        void setSingle(const Word& addr, const float& single);
        void setDouble(const Word& addr, const double& dble);
    };

};

#endif