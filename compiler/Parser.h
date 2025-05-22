#ifndef __PARSER_H__
#define __PARSER_H__
#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include <unordered_map>

namespace Parser {
    using TokenString = std::unique_ptr<std::vector<std::string>>;
    using BinaryInstruction = unsigned int;
    using Byte = char;

    // Split function, similar to python. Breaks when encountering comment #
    TokenString split(const std::string& line);

    BinaryInstruction toInstruction(TokenString& token_string);

    class Compiler {
        static inline bool exists = false;
        std::unique_ptr<std::ifstream> file;

    public:
        std::vector<Byte> data;
        std::vector<BinaryInstruction> text;

        Compiler();
        ~Compiler();

        void loadFile(const std::string& path);
        void dumpFile(const std::string& path);

        const std::unique_ptr<std::ifstream>& getFile() const;
    };

    Byte convertRegNameToNumber(const std::string& reg);
    BinaryInstruction buildRType(const Byte& rd, const Byte& rs, const Byte& rt, const Byte& shamt, const Byte& funct);
    BinaryInstruction buildIType(const Byte& opcode, const Byte& rt, const Byte& rs, const short& imm);
    BinaryInstruction buildJType(const Byte& opcode, const unsigned int& addr);

};

#endif