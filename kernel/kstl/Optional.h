#ifndef M__OPTIONAL_H__
#define M__OPTIONAL_H__
#include "Error.h"

namespace ministl {

    template <class T>
    constexpr T* launder(T* p) noexcept {
    // Attempt to implement std launder if available
    #if defined(__has_builtin)
    #  if __has_builtin(__builtin_launder)
        return __builtin_launder(p);
    #  endif
    #endif
    #if defined(__GNUC__) || defined(__clang__)
    // Older GCC/Clang also have it even if __has_builtin is missing.
    return __builtin_launder(p);
    #else
    // MSVC and others: best-effort no-op
    return p;
    #endif
    }

    struct nullopt_t { explicit constexpr nullopt_t(int) {} };
    struct in_place_t { explicit constexpr in_place_t(int) {} };
    
    inline constexpr nullopt_t nullopt{0};
    inline constexpr in_place_t in_place{0};

    template <size_t N>
    using Buffer = unsigned char[N];

    template <typename T>
    class optional {
        alignas(T) Buffer<sizeof(T)> _storage;
        bool live;

        T* raw_ptr() { return reinterpret_cast<T*>(_storage); }
        const T* raw_ptr() const { return reinterpret_cast<const T*>(_storage); }

        T* ptr() { return ministl::launder(raw_ptr()); }
        const T* ptr() const { return ministl::launder(raw_ptr()); }
    
    public:
        optional() : live(false) {}
        optional(nullopt_t) : live(false) {}

        optional(const T& v) : live(true) { new (raw_ptr()) T(v); }
        optional(T&& v) : live(true) { new (raw_ptr()) T(static_cast<T&&>(v)); }

        template <class... Args>
        explicit optional(in_place_t, Args&&... args) : live(true) {
            new (raw_ptr()) T(static_cast<Args&&>(args)...);
        }

        optional(const optional& other) : live(other.live) {
            if (live) new (raw_ptr()) T(*other.ptr());
        }

        optional(optional&& other) : live(other.live) {
            if (live) new (raw_ptr()) T(static_cast<T&&>(*other.ptr()));
        }

        void reset() {
            if (!live) return;
            raw_ptr()->~T();
            live = false;
        }

        template <class... Args>
        T& emplace(Args&&... args) {
            reset();
            new (raw_ptr()) T(static_cast<Args&&>(args)...);
            live = true;
            return *ptr();
        }

        optional& operator=(nullopt_t) {
            reset();
            return *this;
        }

        optional& operator=(const optional& other) {
            if (this == &other) return *this;
            else if (live && other.live) {
                *ptr() = *other.ptr();
            }
            else if (live && !other.live) {
                reset();
            }
            else if (!live && other.live) {
                emplace( *other.ptr() );
            }

            return *this;
        }

        optional& operator=(const optional&& other) {
            if (this == &other) return *this;
            else if (live && other.live) {
                *ptr() = static_cast<T&&>(*other.ptr());
            }
            else if (live && !other.live) {
                reset();
            }
            else if (!live && other.live) {
                emplace( static_cast<T&&>(*other.ptr()) );
            }

            return *this;
        }

        optional& operator=(const T& v) {
            if (live) *ptr() = v;
            else emplace(v);
            return *this;
        }

        optional& operator=(T&& v) {
            if (live) *ptr() = static_cast<T&&>(v);
            else emplace(static_cast<T&&>(v));
            return *this;
        }

        ~optional() { reset(); }

        bool has_value() const { return live; }
        operator bool() const { return live; }

        T& operator*() & { return *ptr(); }
        const T& operator*() const& { return *ptr(); }

        T&& operator*() && { return static_cast<T&&>(*ptr()); }
        
        T* operator->() { return ptr(); }
        const T* operator->() const { return ptr(); }

        T& value() & {
            assert(live);
            return *ptr();
        }

        const T& value() const& {
            assert(live);
            return *ptr();
        }

        T&& value() && {
            assert(live);
            return static_cast<T&&>(*ptr());
        }

        template <class U>
        T value_or(U&& default_value) const {
            return live ? *ptr() : static_cast<U&&>(default_value);
        }
        
    };

    template <typename T, class... Args>
    optional<T> make_optional(Args&&... args) {
        return optional<T>(in_place, static_cast<Args&&>(args)...);
    }

}

#endif