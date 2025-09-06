#ifndef M__ERROR_H__
#define M__ERROR_H__
#include "asmio.h"

namespace std {
    using size_t = unsigned int;
}

typedef decltype(sizeof(0)) size_t;

inline void assert_fail(const char* cond, const char* file, int line, const char* func) {    
    std::printString("Assertion triggered: \n'");
    std::printString(cond);
    std::printString("'\nfailed in file:\n'");
    std::printString(file);
    std::printString("'\nat line:\n'");
    std::printInteger(line);
    std::printString("'\nin function:\n'");
    std::printString(func);
    std::printString("'\n");
    std::exit();
}

#define __ASSERT_FUNCTION __extension__ __PRETTY_FUNCTION__
#define assert(expr) (static_cast <bool> (expr) ? void (0) : assert_fail(#expr, __FILE__, __LINE__, __ASSERT_FUNCTION))

inline void* operator new(size_t, void* ptr) { return ptr; }

#endif