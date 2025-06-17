#ifndef M__ARRAY_H__
#define M__ARRAY_H__
#include "Error.h"

namespace ministl {

    template <typename T, size_t N>    
    class array {
        T storage[N];
        size_t _size;
        const size_t _capacity = N;

    public:
        array() : _size(0) {}
        
        array(const array<T, N>& other) {
            for (size_t i = 0; i < N; ++i) storage[i] = other.storage[i];
            _size = other._size;
        }

        array<T, N>& operator=(const array<T, N>& other) {
            if (this == &other) return *this;
            for (size_t i = 0; i < N; ++i) storage[i] = other.storage[i];
            _size = other._size;
            return *this;
        }

        array(const T* arr, size_t size) {
            assert(size <= _capacity);
            _size = size;
            for (size_t i = 0; i < size; ++i) storage[i] = arr[i];
        }


        inline size_t size() const { return _size; }
        inline constexpr size_t capacity() const { return _capacity; }
        inline bool empty() const { return _size == 0; }

        T& operator[](size_t index) { assert(index < _size); return storage[index]; }
        const T& operator[](size_t index) const { assert(index < _size); return storage[index]; }
        
        void push_back(const T& value) {
            assert(_size < N);
            storage[_size] = value;
            ++_size;
        }

        void push_back(T&& value) {
            assert(_size < N);
            storage[_size] = value;
            ++_size;
        }

        void pop_back() {
            assert(_size > 0);
            storage[_size - 1].~T();
            --_size;
        }   

        void clear() {
            while (_size > 0) pop_back();
        }

    };

}


#endif