#ifndef M__ERROR_H__
#define M__ERROR_H__
#include "../ASMInterface.h"

typedef decltype(sizeof(0)) size_t;

void assert_fail(const char* cond, const char* file, int line, const char* func) {    
    PrintString("Assertion triggered: \n'");
    PrintString(cond);
    PrintString("'\nfailed in file:\n'");
    PrintString(file);
    PrintString("'\nat line:\n'");
    PrintInteger(line);
    PrintString("'\nin function:\n'");
    PrintString(func);
    PrintString("'\n");
    Halt;
}

#define __ASSERT_FUNCTION __extension__ __PRETTY_FUNCTION__
#define assert(expr) (static_cast <bool> (expr) ? void (0) : assert_fail(#expr, __FILE__, __LINE__, __ASSERT_FUNCTION))




#endif