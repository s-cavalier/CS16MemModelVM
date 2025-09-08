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

        size_t size() const { return _size; }
        bool empty() const { return _size == 0; }

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

            friend class unordered_map;
        };

        iterator begin() {
            return iterator(buckets.begin(), buckets.end(), buckets.empty() ? typename bucket_type::iterator{} : buckets.front().begin());
        }
        iterator end() {
            return iterator(buckets.end(), buckets.end(), typename bucket_type::iterator{});
        }

        class const_iterator {
            using outer_iter = typename BucketContainer::const_iterator;
            using inner_iter = typename bucket_type::const_iterator;

            outer_iter outer;     
            outer_iter outer_end; 
            inner_iter inner;     

        public:
            using iterator_category = ministl::forward_iterator_tag;
            using difference_type   = ministl::ptrdiff_t;
            using pointer           = const value_type*;
            using reference         = const value_type&;

            const_iterator() = default;

            const_iterator(outer_iter o, outer_iter o_end, inner_iter i)
                : outer(o), outer_end(o_end), inner(i) {
                skip_empty();
            }

            reference operator*()  const { return *inner; }
            pointer   operator->() const { return &*inner; }

            const_iterator& operator++() {
                ++inner;
                skip_empty();
                return *this;
            }

            const_iterator operator++(int) {
                const_iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            bool operator==(const const_iterator& other) const {
                return outer == other.outer &&
                    (outer == outer_end || inner == other.inner);
            }
            bool operator!=(const const_iterator& other) const {
                return !(*this == other);
            }

        private:
            void skip_empty() {
                while (outer != outer_end && inner == outer->end()) {
                    ++outer;
                    if (outer != outer_end) inner = outer->begin();
                }
            }

            friend class unordered_map;
        };

        const_iterator begin() const {
            return const_iterator(
                buckets.begin(), buckets.end(),
                buckets.empty() ? typename bucket_type::const_iterator{}
                                : buckets.front().begin());
        }
        const_iterator end() const {
            return const_iterator(
                buckets.end(), buckets.end(),
                typename bucket_type::const_iterator{});
        }

        const_iterator cbegin() const { return begin(); }
        const_iterator cend()   const { return end(); }


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
        pair<iterator, bool> emplace(Args&&... args) {
            if ( _size == buckets.size() ) rehash( buckets.size() * 2 );

            value_type tmp(ministl::forward<Args>(args)...);
            uint32_t loc = hasher(tmp.first) % buckets.size();

            for (auto it = buckets[loc].begin(); it != buckets[loc].end(); ++it) {
                if ( keyEqual(tmp.first, it->first) ) return pair<iterator, bool>( iterator(buckets.begin() + loc, buckets.end(), it) , false);
            }

            ++_size;
            buckets[loc].emplace_back( ministl::move(tmp) ).second;
            return pair<iterator, bool>( iterator( buckets.begin() + loc, buckets.end(), buckets[loc].tail_it() ), true );
        }

        pair<iterator, bool> insert(const value_type& value) {
            return emplace(value);
        }

        pair<iterator, bool> insert(value_type&& value) {
            return emplace(ministl::move(value));
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

        iterator find(const Key& key) {
            uint32_t loc = hasher(key) % buckets.size();
            for (auto it = buckets[loc].begin(); it != buckets[loc].end(); ++it) {
                if (keyEqual(key, it->first)) return iterator(buckets.begin() + loc, buckets.end(), it);
            }

            return end();
        }

        const_iterator find(const Key& key) const {
            uint32_t loc = hasher(key) % buckets.size();
            for (auto it = buckets[loc].cbegin(); it != buckets[loc].cend(); ++it) {
                if (keyEqual(key, it->first)) return const_iterator(buckets.begin() + loc, buckets.end(), it);
            }

            return end();
        }

        bool contains(const Key& key) const {
            uint32_t loc = hasher(key) % buckets.size();
            for (const auto& kv_pair : buckets[loc]) {
                if ( keyEqual(key, kv_pair.first) ) return true;
            }

            return false;
        }

        iterator erase( iterator pos ) {
            if (pos == end()) return end();

            auto next = pos;
            ++next;
            pos.outer->erase( pos.inner );
            return next;
        }

        bool erase( const Key& key ) {
            uint32_t loc = hasher(key) % buckets.size();
            
            for (auto it = buckets[loc].begin(); it != buckets[loc].end(); ++it) {
                if ( keyEqual(key, it->first) ) {
                    buckets[loc].erase(it);
                    return true;
                }
            }

            return false;
        }

    };



}


#endif