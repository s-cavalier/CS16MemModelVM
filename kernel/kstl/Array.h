#ifndef M__ARRAY_H__
#define M__ARRAY_H__
#include "Error.h"

namespace ministl {

    template <typename T, size_t N>    
    class array {
        T storage[N];
        const size_t _capacity = N;

    public:
        array() {}
        
        array(const array<T, N>& other) {
            for (size_t i = 0; i < N; ++i) storage[i] = other.storage[i];
        }

        array<T, N>& operator=(const array<T, N>& other) {
            if (this == &other) return *this;
            for (size_t i = 0; i < N; ++i) storage[i] = other.storage[i];
            return *this;
        }

        array(const T* arr, size_t size) {
            for (size_t i = 0; i < size; ++i) storage[i] = arr[i];
        }

        inline constexpr size_t capacity() const { return _capacity; }

        T& operator[](size_t index) { assert(index < _capacity); return storage[index]; }
        const T& operator[](size_t index) const { assert(index < _capacity); return storage[index]; }

        inline T* data() { return storage; }
        inline const T* data() const { return storage; }

    };

}


#endif