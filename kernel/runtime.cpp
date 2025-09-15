
using size_t = decltype(sizeof(0));

extern "C" void* memcpy(void* dest, const void* src, size_t n) {
    char* d = static_cast<char*>(dest);
    const char* s = static_cast<const char*>(src);
    for (size_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }
    return dest;
}

extern "C" void* memset(void *dest, int c, size_t n) {
    auto p = static_cast<unsigned char*>(dest);
    while (n--) *p++ = static_cast<unsigned char>(c);
    return dest;
}


// TODO WHEN MULTI-THREADED
extern "C" int __cxa_guard_acquire (unsigned long *g)
{
    return !*(volatile unsigned char*)g;
}

extern "C" void __cxa_guard_release (unsigned long *g)
{
    *(volatile unsigned char*)g = 1;
}

extern "C" void __cxa_guard_abort (unsigned long *g)
{
    ++g;
}

// Runtime stubs
// Don't need any kind of cleanup yet
extern "C" {
    void* __dso_handle = &__dso_handle;

    using dtor_func_t = void (*)(void*);
    int __cxa_atexit(dtor_func_t, void*, void*) {
        return 0;
    }

    void __cxa_finalize(void*) {}

}