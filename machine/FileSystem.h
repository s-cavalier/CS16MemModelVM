#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__
#include <vector>
#include <optional>
#include <string>
#include <unordered_map>
#include "Memory.h"

// Temporary (?) for file I/O
// For right now, this is mainly just so the OS can load binaries
// This represents ALL open files. Per-process file tables should be managed by the OS
// Once more OS is built, we can swap this out for just a super rudimentary "disk" operation.

using DoubleWord = unsigned long long;
using Word = unsigned int;
using HalfWord = unsigned short;
using Byte = unsigned char;



namespace Hardware {

    class OpenFile {
        Word fd;
        Word err;
        DoubleWord offset;

    public:
        OpenFile(const std::string& filePath, Word flags);

        std::vector<Byte> read(Word count);
        inline Word error() const { return err; }

        ~OpenFile();

    };


    class FileSystem {
        std::vector<std::optional<OpenFile>> files;
        std::vector<Word> freeVFDs;

    public:
        FileSystem();

        Word open(const std::string& filePath, Word flags);
        int close(Word vfd);

        std::optional<OpenFile>& operator[](Word vfd);
        const std::optional<OpenFile>& operator[](Word vfd) const;
    };

}




#endif