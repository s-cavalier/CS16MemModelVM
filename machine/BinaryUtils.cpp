#include "BinaryUtils.h"
#include <cstring>

unsigned int Binary::loadBigEndian(const unsigned char bytes[4]) {
    return (((unsigned int)(bytes[0])) << 24) |
        (((unsigned int)(bytes[1])) << 16) |
        (((unsigned int)(bytes[2])) << 8)  |
        (((unsigned int)(bytes[3]))); 
    }

