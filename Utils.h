#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

typedef unsigned char BYTE;
typedef unsigned short HALFWORD;
typedef unsigned int WORD;

namespace Hardware {
    typedef std::vector<std::string> token_string;

    struct lab6_branch_info {
        WORD pc;
        bool took_branch;
        bool is_branch;

        std::string unpack() const;
    };

    struct CPU {
        bool exit_flag = false;

        int rf[32] = {0};
        WORD pc = 0;
        WORD END_OF_PROGRAM = -1;
        
        std::unordered_map<WORD, int> d_mem;
        std::unordered_map<std::string, WORD> labels;
        std::vector<token_string> i_mem;

        lab6_branch_info run_instruction();                  // Run instruction at i_mem[pc]. Current implementation returns branch info for lab 6.
    };

}

namespace Parser {
    typedef std::vector<std::string> token_string;
    typedef bool (*continue_if)(const std::string&);
    typedef bool (*break_if)(const std::string&);

    // Check for instruction existing (case matters!)
    bool valid_instruction(const std::string& instr);

    // Convert from register -> number
    BYTE reg_name_to_index(const std::string& reg);

    WORD instr_to_hex(token_string& instr, const std::unordered_map<std::string, WORD>& label_table, const WORD& tmp_pc);

    WORD build_r_type(const BYTE& rd, const BYTE& rs, const BYTE& rt, const BYTE& funct);
    WORD build_i_type(const BYTE& opcode, const BYTE& rs, const BYTE& rt, const short& imm);

    std::string get_first_token(const std::string& line);

    token_string split(const std::string& line);

}

namespace Utils {
    void error(const std::string& exit_msg);
    
    void assert(const bool& good_eval, const std::string& on_error);
}