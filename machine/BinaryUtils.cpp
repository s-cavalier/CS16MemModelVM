#include "BinaryUtils.h"
#include <cstring>

unsigned int Binary::loadBigEndian(const unsigned char bytes[4]) {
    return (((unsigned int)(bytes[0])) << 24) |
        (((unsigned int)(bytes[1])) << 16) |
        (((unsigned int)(bytes[2])) << 8)  |
        (((unsigned int)(bytes[3]))); 
    }

// in future, utilize machine big endian-ness for speedups since we can directly memcpy

void Binary::storeDoubleBE(float* loc, const double& dble) {
    unsigned long long bits;
    std::memcpy(&bits, &dble, sizeof(bits));
    
    unsigned int hi = static_cast<unsigned int>(bits >> 32);
    unsigned int lo = static_cast<unsigned int>(bits & 0xFFFFFFFFu);
    
    float fhi, flo;
    std::memcpy(&fhi, &hi, sizeof(fhi));
    std::memcpy(&flo, &lo, sizeof(flo));
    
    loc[0] = fhi;
    loc[1] = flo;
}


double Binary::loadDoubleBE(const float* vals) {
    float fhi = vals[0];
    float flo = vals[1];

    unsigned int hi, lo;
    std::memcpy(&hi, &fhi, sizeof(hi));
    std::memcpy(&lo, &flo, sizeof(lo));

    unsigned long long bits = (static_cast<unsigned long long>(hi) << 32) | lo;

    double result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}