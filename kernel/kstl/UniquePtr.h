#ifndef M__UNIQUE_PTR_H__
#define M__UNIQUE_PTR_H__
#include "Error.h"

namespace ministl {

    template<typename T> struct remove_reference { using type = T; };
    template<typename T> struct remove_reference<T&> { using type = T; };
    template<typename T> struct remove_reference<T&&> { using type = T; };

    template<typename T>
    constexpr T&& forward(typename remove_reference<T>::type& t) {
        return static_cast<T&&>(t);
    }

    template<typename T>
    constexpr T&& forward(typename remove_reference<T>::type&& t) {
        return static_cast<T&&>(t);
    }

    struct true_type  { static constexpr bool value = true;  };
    struct false_type { static constexpr bool value = false; };

    template<bool B, class T = void> struct enable_if {};
    template<class T> struct enable_if<true, T> { using type = T; };

    // declval
    template<class T> T&& declval() noexcept;

    // is_convertible<From, To> (general; works fine for pointers)
    template<class From, class To>
    struct is_convertible {
    private:
        static true_type  test(To);
        static false_type test(...);
        static From make();
    public:
        static constexpr bool value = decltype(test(make()))::value;
    };


    template<typename T>
    class unique_ptr {
        T* ptr;
    public:
        explicit unique_ptr(T* p = nullptr) : ptr(p) {}
        ~unique_ptr() { if (ptr) delete ptr; }

        unique_ptr(unique_ptr&& other) : ptr(other.ptr) { other.ptr = nullptr; }
        unique_ptr& operator=(unique_ptr&& other) {
            if (this != &other) {
                delete ptr;
                ptr = other.ptr;
                other.ptr = nullptr;
            }
            return *this;
        }

        // Converting move constructor: unique_ptr<U> -> unique_ptr<T> if U* -> T*
        template<class U, class = typename enable_if<is_convertible<U*, T*>::value>::type>
        unique_ptr(unique_ptr<U>&& other) noexcept
            : ptr(other.release()) {}

        // Converting move assignment: unique_ptr<U> -> unique_ptr<T> if U* -> T*
        template<class U, class = typename enable_if<is_convertible<U*, T*>::value>::type>
        unique_ptr& operator=(unique_ptr<U>&& other) noexcept {
            reset(other.release());
            return *this;
        }

        inline T* get() const { return ptr; }
        inline T* operator->() const { return ptr; }
        inline T& operator*() const { return *ptr; }

        inline T* release() { T* ret = ptr; ptr = nullptr; return ret; }

        unique_ptr(const unique_ptr&) = delete;
        unique_ptr& operator=(const unique_ptr&) = delete;
    };

    template<typename T, typename... Args>
    unique_ptr<T> make_unique(Args&&... args) {
        return unique_ptr<T>(new T(forward<Args>(args)...));
    }

    template<typename T>
    constexpr typename remove_reference<T>::type&& move(T&& t) {
        return static_cast<typename remove_reference<T>::type&&>(t);
    }

    template<typename T>
    class unique_ptr<T[]> {
        T* ptr;
    public:
        explicit unique_ptr(T* p = nullptr) : ptr(p) {}
        ~unique_ptr() { if (ptr) delete[] ptr; }

        unique_ptr(unique_ptr&& other) : ptr(other.ptr) { other.ptr = nullptr; }
        unique_ptr& operator=(unique_ptr&& other) {
            if (this != &other) {
                delete[] ptr;
                ptr = other.ptr;
                other.ptr = nullptr;
            }
            return *this;
        }

        T* get() const { return ptr; }
        T& operator[](size_t i) const { return ptr[i]; }

        unique_ptr(const unique_ptr&) = delete;
        unique_ptr& operator=(const unique_ptr&) = delete;
    };

    template<typename T>
    ministl::unique_ptr<T[]> make_unique(size_t n) {
        return ministl::unique_ptr<T[]>(new T[n]());
    }




}




#endif