#include "Parser.h"
#include <sstream>
#include <stdexcept>

Parser::TokenString Parser::split(const std::string& line) {
    std::istringstream iss(line);
    std::vector<std::string>* token_string = new std::vector<std::string>;

    for (std::string token; iss >> token;) token_string->push_back(token);

    return TokenString(token_string);
};

Parser::BinaryInstruction Parser::toInstruction(TokenString& token_string) {
    typedef BinaryInstruction (*toBinary)(TokenString& token_string);

    static const std::unordered_map<std::string, toBinary> switchCase = {
        {"nop", [] (TokenString& token_string) -> BinaryInstruction { return 0; }},  // demo line
        // R-Type
        {"add", [] (TokenString& token_string) -> BinaryInstruction {
            Byte rd = convertRegNameToNumber(token_string->at(1));
            Byte rs = convertRegNameToNumber(token_string->at(2));
            Byte rt = convertRegNameToNumber(token_string->at(3));
            return buildRType(rd, rs, rt, 0, 0x20); 
        }}
    };

    auto it = switchCase.find(token_string->front());
    if (it == switchCase.end()) throw std::runtime_error("Encountered unrecognized instruction: " + token_string->front());
    return it->second(token_string);
}

// ---------------------------
// Assert Singleton Properties
// ---------------------------

Parser::Compiler::Compiler() {
    if (exists) throw std::runtime_error(">1 instance of Compiler is not allowed.");
    exists = true;
}

Parser::Compiler::~Compiler() {
    exists = false;
}

const std::unique_ptr<std::ifstream>& Parser::Compiler::getFile() const {
    return file;
}

// ----------
// Core Logic
// ----------

void Parser::Compiler::loadFile(const std::string& path) {
    file.reset(new std::ifstream(path));
    if (file->fail()) return;




};

void Parser::Compiler::dumpFile(const std::string& path) {
    
};

Parser::Byte Parser::convertRegNameToNumber(const std::string& reg) {
    static const std::unordered_map<std::string, Byte> nameToReg = {
        {"$zero", 0},
        {"$at", 1},
        {"$v0", 2},
        {"$v1", 3},
        {"$a0", 4},
        {"$a1", 5},
        {"$a2", 6},
        {"$a3", 7},
        {"$t0", 8},
        {"$t1", 9},
        {"$t2", 10},
        {"$t3", 11},
        {"$t4", 12},
        {"$t5", 13},
        {"$t6", 14},
        {"$t7", 15},
        {"$s0", 16},
        {"$s1", 17},
        {"$s2", 18},
        {"$s3", 19},
        {"$s4", 20},
        {"$s5", 21},
        {"$s6", 22},
        {"$s7", 23},
        {"$t8", 24},
        {"$t9", 25},
        {"$k0", 26},
        {"$k1", 27},
        {"$gp", 28},
        {"$sp", 29},
        {"$fp", 30},
        {"$ra", 31}
    };

    if (reg[1] == '0') return 0;    // atoi(x) returns 0 if x cannot be converted, so check if the value is 0

    Byte conversion = Byte(atoi(reg.c_str() + 1)); 
    if (conversion) return conversion;

    auto it = nameToReg.find(reg);
    if (it != nameToReg.end()) return it->second;

    throw std::runtime_error("Encountered invalid register " + reg);
}