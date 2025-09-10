#ifndef M__ASMIO_H__
#define M__ASMIO_H__

namespace std {
    
    void exit();
    void printInteger(const int& i);
    int readInteger();
    void printString(const char* str);
    
    unsigned int fork();
    void exec(const char* file);

};

#endif