#ifndef M__INIT_LIST_H__
#define M__INIT_LIST_H__

// Has to be std:: for compiler to recognize
namespace std {

    template <typename T>
    class initializer_list {
    public:
        using value_type      = T;
        using reference       = const T&;
        using const_reference = const T&;
        using size_type       = size_t;
        using iterator        = const T*;
        using const_iterator  = const T*;

    private:
        iterator _begin;
        size_type _size;

        // compiler calls this constructor
        constexpr initializer_list(const_iterator b, size_type s)
            : _begin(b), _size(s) {}

    public:
        constexpr initializer_list() noexcept : _begin(nullptr), _size(0) {}

        constexpr size_type size() const noexcept { return _size; }
        constexpr const_iterator begin() const noexcept { return _begin; }
        constexpr const_iterator end() const noexcept { return _begin + _size; }
    };

    // deduction guides
    template<class T>
    constexpr const T* begin(initializer_list<T> il) noexcept {
        return il.begin();
    }
    template<class T>
    constexpr const T* end(initializer_list<T> il) noexcept {
        return il.end();
    }


}


#endif