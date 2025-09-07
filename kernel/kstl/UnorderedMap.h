#ifndef M__UNORDERED_MAP_H__
#define M__UNORDERED_MAP_H__
#include "MyStdInt.h"
#include "Queue.h"
#include "Vector.h"
#include "String.h"
#include "Iterator.h"

namespace ministl {

    template <typename T>
    struct hash;

    template <>
    struct hash<uint32_t> {
        uint32_t operator()(uint32_t h) const {
            h ^= h >> 16;
            h *= 0x85ebca6b;
            h ^= h >> 13;
            h *= 0xc2b2ae35;
            h ^= h >> 16;
            return h;
        }
    };

    template <>
    struct hash<int32_t> {
        uint32_t operator()(int32_t h) const {
            h ^= h >> 16;
            h *= 0x85ebca6b;
            h ^= h >> 13;
            h *= 0xc2b2ae35;
            h ^= h >> 16;
            return h;
        }
    };

    template <typename T>
    struct hash<T*> {
        uint32_t operator()(T* h) const {
            h ^= h >> 16;
            h *= 0x85ebca6b;
            h ^= h >> 13;
            h *= 0xc2b2ae35;
            h ^= h >> 16;
            return h;
        }
    };

    template <>
    struct hash<const char*> {
        uint32_t operator()(const char* str) const {
            uint32_t hash = 2166136261u;
            while (*str) {
                hash ^= (uint8_t)(*str++);
                hash *= 16777619u;      
            }
            return hash;
        }
    };

    // Could introduce a string overload

    template <typename T>
    struct equal_to {
        bool operator()(const T& a, const T& b) const {
            return a == b;
        }
    };

    template <>
    struct equal_to<const char*> {
        bool operator()(const char* a, const char* b) const {
            return ministl::streq(a, b);
        }
    };

    template <typename T1, typename T2>
    struct pair {
        T1 first;
        T2 second;

        pair() : first(), second() {}

        template <typename U1, typename U2>
        pair(U1&& f, U2&& s) : first(ministl::forward<U1>(f)), second(ministl::forward<U2>(s)) {}
        pair(const pair&) = default;
        pair(pair&&) = default;
        pair& operator=(const pair&) = default;
        pair& operator=(pair&&) = default;
        ~pair() = default;

    };


    // Implement load factor when more floating point type scaffolding in place
    // otherwise just use 1 as load factor

    template <typename Key, typename T, typename Hash = hash<Key>, typename KeyEqual = equal_to<Key>>
    class unordered_map {
        using value_type = pair<const Key, T>;
        using bucket_type = list<value_type>;
        using BucketContainer = vector< bucket_type >;

        BucketContainer buckets;
        size_t _size;
        Hash hasher;
        KeyEqual keyEqual;
    
    public:
        unordered_map() : buckets( DEFAULT_CAP ), _size(0) {}
        explicit unordered_map(size_t bucket_count) : buckets( bucket_count ), _size(0) {}
        unordered_map(const unordered_map& other) : buckets(other.buckets), _size(other._size) {}
        unordered_map( unordered_map&& other ) : buckets( ministl::move(other.buckets) ), _size(other._size) { other._size = 0; }
        unordered_map& operator=( const unordered_map& other ) { 
            if (this == &other) return *this; 
            buckets = other.buckets;
            _size = other._size; 
            return *this; 
        }
        unordered_map& operator=( unordered_map&& other ) { 
            if (this == &other) return *this; 
            buckets = ministl::move(other.buckets); 
            _size = other._size;
            other._size = 0;
            return *this; 
        }
        ~unordered_map() = default;

        class iterator {
            using outer_iter = typename BucketContainer::iterator;
            using inner_iter = typename bucket_type::iterator;

            outer_iter outer;   // which bucket
            outer_iter outer_end;
            inner_iter inner;   // which node inside bucket

        public:
            iterator() = default;
            iterator(outer_iter o, outer_iter o_end, inner_iter i)
                : outer(o), outer_end(o_end), inner(i) {
                skip_empty();
            }

            value_type& operator*()  { return *inner; }
            value_type* operator->() { return &*inner; }

            iterator& operator++() {
                ++inner;
                skip_empty();
                return *this;
            }

            bool operator==(const iterator& other) const {
                return outer == other.outer && 
                    (outer == outer_end || inner == other.inner);
            }
            bool operator!=(const iterator& other) const {
                return !(*this == other);
            }

        private:
            void skip_empty() {
                while (outer != outer_end && inner == outer->end()) {
                    ++outer;
                    if (outer != outer_end) inner = outer->begin();
                }
            }
        };

        iterator begin() {
            return iterator(buckets.begin(), buckets.end(),
                            buckets.empty() ? typename bucket_type::iterator{} 
                                            : buckets.front().begin());
        }
        iterator end() {
            return iterator(buckets.end(), buckets.end(),
                            typename bucket_type::iterator{});
        }

        // Rehashes such that load_factor < 1
        // Thus the bucket count will be at least count
        void rehash(size_t count) {
            if (count < _size) return;

            BucketContainer realloc(count);

            for (size_t i = 0; i < buckets.size(); ++i) {

                while (!buckets[i].empty()) {
                    auto& pop = buckets[i].back();
                    uint32_t loc = hasher(pop.first) % realloc.size();

                    realloc[loc].emplace_back( ministl::move(pop) );
                    buckets[i].pop_back();
                }

            }

            buckets = ministl::move(realloc);
        }

        template <class... Args>
        T& emplace(Args&&... args) {
            if ( _size == buckets.size() ) rehash( buckets.size() * 2 );

            value_type tmp(ministl::forward<Args>(args)...);
            uint32_t loc = hasher(tmp.first) % buckets.size();

            for (auto& kv_pair : buckets[loc] ) {
                if ( keyEqual(tmp.first, kv_pair.first) ) return kv_pair.second;
            }

            ++_size;
            return buckets[loc].emplace_back( ministl::move(tmp) ).second;
        }

        void insert(const value_type& value) {
            emplace(value);
        }

        void insert(value_type&& value) {
            emplace(ministl::move(value));
        }

        unordered_map(std::initializer_list<value_type> il) : buckets( il.size() * 2 ) {
            for (const auto& v : il) emplace(v);
        }

        T& operator[](const Key& key) {
            if ( _size == buckets.size() ) rehash( buckets.size() * 2 );

            uint32_t loc = hasher(key) % buckets.size();
            
            for (auto& kv_pair : buckets[loc]) {
                if ( keyEqual(key, kv_pair.first) ) return kv_pair.second;
            }

            ++_size;
            return buckets[loc].emplace_back( key, T() ).second;
        }
        
        T& operator[](Key&& key) {
            if ( _size == buckets.size() ) rehash( buckets.size() * 2 );

            Key rval(ministl::move(key));
            uint32_t loc = hasher(key) % buckets.size();

            for (auto& kv_pair : buckets[loc]) {
                if ( keyEqual(rval, kv_pair.first) ) return kv_pair.second;
            }

            ++_size;
            return buckets[loc].emplace_back(ministl::move(rval), T()).second;
        }


        // Replace with iterator later
        const T* find(const Key& key) const {
            uint32_t loc = hasher(key) % buckets.size();
            for (const auto& kv_pair : buckets[loc]) {
                if (keyEqual(key, kv_pair.first)) return &kv_pair.second;
            }

            return nullptr;
        }

    };



}


#endif