#include "String.h"


bool ministl::streq(const char* a, const char* b) {
    if (a == b) {
        // same pointer (also covers both NULL)
        return true;
    }
    if (a == nullptr || b == nullptr) {
        // exactly one is NULL
        return false;
    }
    // compare characters one by one
    while (*a != '\0' && *b != '\0') {
        if (*a != *b) {
            return false;
        }
        a++;
        b++;
    }
    // only equal if both hit '\0' at the same time
    return *a == *b;

}
