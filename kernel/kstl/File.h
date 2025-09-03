#ifndef K__FILE_H__
#define K__FILE_H__
#include "../ASMInterface.h"

namespace kernel {
    
    class File {
        VMPackage fileIO;
        bool bad;

    public:
        File(const char* file, uint32_t flags);
        ~File();

        uint32_t read(char* buffer, uint32_t bytes);
        unsigned int seek(uint32_t offset, int whence);
    };
    

}


#endif