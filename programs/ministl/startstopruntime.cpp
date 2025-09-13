#include "asmio.h"

extern int main();

extern "C" {
    typedef void (*Constructor)();
    extern Constructor __init_array_start[];
    extern Constructor __init_array_end[];
}

extern "C" char* _bss_start;
extern "C" char* _end;

extern "C" void bootuserprog() {
    for (char* curr = _bss_start; curr < _end; ++curr) *curr = 0; 

    for (Constructor* func = __init_array_start; func < __init_array_end; ++func) {
        (*func)();
    }


    main();


    std::exit();
}