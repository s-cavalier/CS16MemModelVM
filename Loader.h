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
        bool _bad;

    public:
        Parser();
        
        Section<Byte>& readData();
        Section<Word>& readText();
        
        // "bad bit". Should only be used if file could be opened.
        const bool& bad() const;
    };

    struct ExecutableParser : public Parser {
        ExecutableParser(const std::string& path);
    };

    struct AssemblyParser : public Parser {     // todo
        AssemblyParser(const std::string& path);
    };

}



#endif