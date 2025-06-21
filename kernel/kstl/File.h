#ifndef K__FILE_H__
#define K__FILE_H__
#include "../ASMInterface.h"
#include "Vector.h"

namespace kernel {
    
    class File {
        VMPackage fileIO;
        bool bad;

    public:
        File(const char* file, uint32_t flags);
        ~File();

        ministl::vector<char> read(uint32_t bytes = 0xFFFFFFFF);
        unsigned int seek(uint32_t offset, int whence);
    };
    

}


#endif