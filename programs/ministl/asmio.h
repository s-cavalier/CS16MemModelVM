#ifndef M__ASMIO_H__
#define M__ASMIO_H__

using size_t = decltype(sizeof(0));

namespace std {
    
    void exit();
    void printInteger(const int& i);
    int readInteger();
    void printString(const char* str);
    size_t readString(char* buf, size_t numbytes);
    
    unsigned int fork();
    void exec(const char* file);

};

#endif