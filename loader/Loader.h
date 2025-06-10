#ifndef __LOADER_H__
#define __LOADER_H__
#include <vector>
#include <string>

namespace FileLoader {
    using Word = unsigned int;
    using Byte = unsigned char;
    
    template <typename T>
    using Section = std::vector<T>;

    class Parser {
    protected:
        Section<Byte> data;
        Section<Word> text;
        Word entry;
        bool _bad;

    public:
        Parser();
        
        Section<Byte>& readData();
        Section<Word>& readText();
        Word& readEntry();
        
        // "bad bit". Should only be used if file could be opened.
        const bool& bad() const;
    };

    struct SpimLoader : public Parser {
        SpimLoader(const std::string& path);
    };

    struct ELFLoader : public Parser {
        ELFLoader(const std::string& path);
    };

    class KernelLoader : public Parser {
        Word trapHandlerLocation;

    public:
        inline const Word& getTrapHandlerLocation() const { return trapHandlerLocation; }
        KernelLoader(const std::string& path);
    };

}



#endif