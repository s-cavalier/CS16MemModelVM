#ifndef M__VECTOR_H__
#define M__VECTOR_H__
#include "Error.h"
#include "UniquePtr.h"
#include "InitializerList.h"

namespace ministl {    
    template <typename T>
    class vector {

        size_t _size;
        size_t _capacity;
        T* _data;
        
    public:

        static constexpr size_t DEFAULT_CAP = 8;
        static constexpr size_t GROWTH_FACTOR = 2;

        constexpr size_t roundToNearestPowOf2(size_t x) {
            if (x == 0) return 1;
            --x;
            x |= x >> 1;
            x |= x >> 2;
            x |= x >> 4;
            x |= x >> 8;
            x |= x >> 16;
            return x + 1;
        }

        // Default constructor
        vector() : _size(0), _capacity(DEFAULT_CAP), _data(new T[_capacity]) {}
        
        // Default insert count items
        explicit vector(size_t count) {
            _capacity = roundToNearestPowOf2(count);
            _size = count;
            _data = (T*)::operator new[]( sizeof(T) * _capacity );

            for (size_t i = 0; i < _size; ++i) new (_data + i) T();
        }

        explicit vector(size_t count, const T& value) {
            _capacity = roundToNearestPowOf2(count);
            _size = count;
            _data = (T*)::operator new[]( sizeof(T) * _capacity );

            for (size_t i = 0; i < _size; ++i) new (_data + i) T(value);
        }

        vector(const vector& other) {
            _capacity = other._capacity;
            _size = other._size;
            _data = (T*)::operator new[]( sizeof(T) * _capacity );

            for (size_t i = 0; i < _size; ++i) new (_data + i) T( other._data[i] );
        }

        vector(vector&& other) {
            _capacity = other._capacity;
            _size = other._size;
            _data = other._data;

            other._data = nullptr; // take ownership of memory
            other._capacity = 0;
            other._size = 0;
        }

        vector(std::initializer_list<T> il) {
            _size = il.size();
            _capacity = roundToNearestPowOf2(_size);

            _data = (T*)::operator new[]( sizeof(T) * _capacity );
            size_t i = 0;
            for (const auto& v : il) {
                new (_data + i) T(v);
                ++i;
            }
        }

        void clear() {
            for (size_t i = 0; i < _size; ++i) _data[i].~T();
            _size = 0;
        }

        vector& operator=(vector&& other) {
            if (this == &other) return *this;

            clear();
            ::operator delete[](_data);

            _capacity = other._capacity;
            _size = other._size;
            _data = other._data;

            other._data = nullptr;
            other._capacity = 0;
            other._size = 0;

            return *this;
        }

        vector& operator=(const vector& other) {
            if (this == &other) return *this;

            if (other._size > _capacity) {
                clear();
                ::operator delete[](_data);
                _capacity = other._capacity;
                _data = (T*)::operator new[](sizeof(T) * _capacity);
                _size = 0;
            }

            size_t i = 0;
            for (; i < _size && i < other._size; ++i) {
                _data[i] = other._data[i];
            }

            for (; i < other._size; ++i) {
                new (_data + i) T(other._data[i]);
            }

            for (; i < _size; ++i) {
                _data[i].~T();
            }

            _size = other._size;
            return *this;
        }

        T& operator[](size_t idx) {
            assert(idx < _size);
            return _data[idx];
        }

        const T& operator[](size_t idx) const {
            assert(idx < _size);
            return _data[idx];
        }

        T& at(size_t idx) {
            assert(idx < _size);
            return _data[idx];
        }

        const T& at(size_t idx) const {
            assert(idx < _size);
            return _data[idx];
        }

        T* data() { return _data; }
        const T* data() const { return _data; }

        T& front() {
            assert(_size > 0);
            return _data[0];
        }

        const T& front() const {
            assert(_size > 0);
            return _data[0];
        }

        T& back() {
            assert(_size > 0);
            return _data[_size - 1];
        }

        const T& back() const {
            assert(_size > 0);
            return _data[_size - 1];
        }

        bool empty() const { return _size == 0; }
        operator bool() const { return _size > 0; }

        size_t size() const { return _size; }
        size_t capacity() const { return _capacity; }

        // Reserves an amount of memory >= count, given count > capacity
        void reserve(size_t count) {
            if (count == 0) count = 1;
            if (count <= _capacity) return;
            _capacity = roundToNearestPowOf2(count);

            T* replacement = (T*)::operator new[](sizeof(T) * _capacity);
            for (size_t i = 0; i < _size; ++i) {
                new (replacement + i) T(ministl::move(_data[i]));
                _data[i].~T();
            }

            ::operator delete[](_data);

            _data = replacement;
        }

        // Reserves to the nearest power of two such that capacity > size
        void shrink_to_fit() {
            size_t new_cap = roundToNearestPowOf2(_size);
            if (new_cap >= _capacity) return;
            _capacity = new_cap;
            
            T* replacement = (T*)::operator new[](sizeof(T) * _capacity);
            for (size_t i = 0; i < _size; ++i) {
                new (replacement + i) T(ministl::move(_data[i]));
                _data[i].~T();
            }

            ::operator delete[](_data);

            _data = replacement;
        }

        template <class... Args>
        T& emplace_back(Args&&... args) {
            if (_size == _capacity) reserve(_capacity * GROWTH_FACTOR);

            new (_data + _size) T(ministl::forward<Args>(args)...);
            return _data[_size++];
        }

        void push_back(const T& value) {
            emplace_back(value);
        }

        void push_back(T&& value) {
            emplace_back(ministl::move(value));
        }

        void pop_back() {
            assert(_size > 0);

            _data[--_size].~T();
        }

        void resize(size_t count) {
            while (_size > count) pop_back();
            while (_size < count) emplace_back();
        }

        void resize(size_t count, const T& value) {
            while (_size > count) pop_back();
            while (_size < count) emplace_back(value);
        }

        ~vector() {
            clear();
            if (_data) ::operator delete[](_data);
        }

    };

} 




#endif