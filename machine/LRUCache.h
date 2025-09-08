#ifndef __LRU_CACHE_H__
#define __LRU_CACHE_H__
#include <cstdint>
#include <list>
#include <unordered_map>
#include <stdexcept>

// Basic generic LRUCache class
// Current use is for the instruction cache
// Make it templated just in case

using size_t = decltype(sizeof(0));

template <typename Key, typename T, size_t CAPACITY = UINT32_MAX>
class LRUCache {
    using value_type = std::pair<const Key, T>;
    using Storage = std::list< value_type >;
    using StorageIt = typename Storage::iterator;

    Storage storage;
    std::unordered_map<Key, StorageIt> tracker;

public:

    LRUCache() {}
    explicit LRUCache( size_t bucket_count ) : tracker(bucket_count) {}
    LRUCache(const LRUCache& other) : storage( other.storage ), tracker( other.tracker ) {}
    LRUCache(LRUCache&& other) : storage( std::move( other.storage ) ), tracker( std::move( other.tracker ) ) {}
    LRUCache& operator=(const LRUCache& other) {
        if (this == &other) return *this;
        storage = other.storage;
        tracker = other.tracker;
        return *this;
    }
    LRUCache& operator=(LRUCache&& other) {
        if (this == &other) return *this;
        storage = std::move(other.storage);
        tracker = std::move(other.tracker);
        return *this;
    }
    ~LRUCache() { clear(); }

    bool empty() const { return storage.empty(); }
    size_t size() const { return storage.size(); }

    void clear() {
        tracker.clear();
        storage.clear();
    }

    void rehash(size_t n) {
        tracker.rehash(n);
    }

    using iterator       = typename std::list< std::pair<const Key, T> >::iterator;
    using const_iterator = typename std::list< std::pair<const Key, T> >::const_iterator;
    iterator begin() { return storage.begin(); }
    iterator end()   { return storage.end();   }
    const_iterator begin() const { return storage.begin(); }
    const_iterator end()   const { return storage.end();   }
    const_iterator cbegin() const { return storage.cbegin(); }
    const_iterator cend()   const { return storage.cend();   }

private:
    void touch(iterator node) {
        storage.splice(storage.begin(), storage, node); // O(1) move-to-front
    }
public:

    template<class... Args>
    std::pair<iterator,bool> emplace(Key k, Args&&... args) {
        auto mit = tracker.find(k);
        if (mit != tracker.end()) {
            mit->second->second = T(std::forward<Args>(args)...);
            touch(mit->second);
            return { mit->second, false }; 
        }

        if constexpr (CAPACITY == 0) {
            return { storage.end(), false }; 
        }
        if (tracker.size() == CAPACITY) {
            auto& kv = storage.back();
            tracker.erase(kv.first);
            storage.pop_back();
        }

        storage.emplace_front(std::move(k), T(std::forward<Args>(args)...));
        tracker[storage.front().first] = storage.begin();
        return { storage.begin(), true };
    }


    std::pair<iterator, bool> insert( const value_type& value ) {
        return emplace(value);
    }

    std::pair<iterator, bool> insert( value_type&& value ) {
        return emplace( std::move(value) );
    }

    LRUCache(std::initializer_list<value_type> li) : tracker( li.size() * 2 ) {
        for (const auto& [k, v] : li ) emplace(k, v);
    }

    T& operator[](const Key& k) {
        auto mit = tracker.find(k);
        if (mit != tracker.end()) {
            touch(mit->second);
            return mit->second->second;
        }

        if constexpr (CAPACITY == 0) {
            throw std::length_error("LRUCache capacity is zero");
        }
        if (tracker.size() == CAPACITY) {
            auto& kv = storage.back();
            tracker.erase(kv.first);
            storage.pop_back();
        }
        storage.emplace_front(k, T{});
        tracker[storage.front().first] = storage.begin();
        return storage.front().second;
    }

    T& at(const Key& k) {
        auto mit = tracker.find(k);
        if (mit == tracker.end()) {
            throw std::out_of_range("LRUCache::at: key not found");
        }
        touch(mit->second);
        return mit->second->second;
    }

    const T& at(const Key& k) const {
        auto mit = tracker.find(k);
        if (mit == tracker.end()) {
            throw std::out_of_range("LRUCache::at: key not found");
        }
        return mit->second->second;
    }

    iterator find(const Key& k) {
        auto mit = tracker.find(k);
        if (mit == tracker.end()) return storage.end();
        return mit->second;
    }

    const_iterator find(const Key& k) const {
        auto mit = tracker.find(k);
        if (mit == tracker.end()) return storage.end();
        return mit->second;
    }

    bool contains(const Key& key) const {
        return tracker.find(key) != tracker.end();
    }

    void pop() {
        if (storage.empty()) return;
        tracker.erase(storage.back().first);
        storage.pop_back();
    }



};

#endif