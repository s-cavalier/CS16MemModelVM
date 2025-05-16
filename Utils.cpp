#ifndef UTILS_H
#define UTILS_H

#include "Utils.h"
#include <iostream>
#include <iomanip>
#include <sstream>
using namespace std;

string Hardware::lab6_branch_info::unpack() const {
    ostringstream oss;
    oss << "0x" << hex << setw(8) << setfill('0') << pc << ": ";
    oss << took_branch << " : " << is_branch;
    return oss.str();
}

Hardware::lab6_branch_info Hardware::CPU::run_instruction() {
    token_string& instr = i_mem[pc];
    const string& instr_type = instr.front();


    lab6_branch_info ret = { pc * 4, false, false };                // took_branch needs to be figured out later

    // SPECIAL INSTRUCTIONS BEGIN

    if (instr_type == "nop") return ret;

    if (instr_type == "li") {
        Utils::assert(instr.size() == 3, "Found bad syntax for " + instr_type + ".");
        int tmp_imm = (int)stoll(instr[2]);
        BYTE arg0 = Parser::reg_name_to_index(instr[1]);

        rf[arg0] = tmp_imm;
        return ret;
    }

    if (instr_type == "move") {

        Utils::assert(instr.size() == 3, "Found bad syntax for " + instr_type + ".");
        BYTE arg0 = Parser::reg_name_to_index(instr[1]);
        BYTE arg1 = Parser::reg_name_to_index(instr[2]);

        rf[arg0] = rf[arg1];
        return ret;
    }

    if (instr_type == "j") {
        Utils::assert(instr.size() == 2, "Found bad syntax for " + instr_type + ".");
        Utils::assert(labels.count(instr[1]), "Couldn't find label " + instr[1] + " for instruction j.");
        pc = labels[instr[1]] - 1;

        ret.is_branch = true;
        ret.took_branch = true;
        return ret;
    }

    if (instr_type == "syscall") {
        Utils::assert(instr.size() == 1, "Found bad syntax for syscall. Arguments should be in $v0 or $a0, accordingly.");
        int& v0 = rf[2];
        int& a0 = rf[4];
        int& a1 = rf[5];

        switch (v0) {
            case 1:
                // Print integer in $a0
                cout << a0;
                break;
            case 4:
                // Print string in $a0
                for (WORD i = 0; d_mem[a0 + i] != 0; ++i) cout << char(d_mem[a0 +i]);
                break;
            case 5: 
            {   // Read integer input
                string val;
                cin >> val;
                v0 = stoi(val);
                break;
            }
            case 8: 
            {   // Read string to $a0 with length $a1
                string val;
                getline(cin, val);
                Utils::assert(int(val.size()) < a1, "Buffer overflow");
                WORD i = 0;
                for (const auto& c : val) d_mem[a0 + i++] = c;
                d_mem[a0 + i] = 0;
                break;
            }
            case 10:
                // Exit
                pc = 4294967290;
                break;
            case 11:
                // Print char
                cout << char(a0);
                break;
            case 12: 
            {   // Read char
                string val;
                cin >> val;
                v0 = int(val.front());
                break;
            }
            default:
                cout << "WARNING: Unsupported syscall code " + to_string(v0) + ".\n";
                break;
        }
        return ret;
    }

    // SPECIAL INSTRUCTIONS END

    BYTE arg0 = Parser::reg_name_to_index(instr[1]);

    // R-type
    if (instr_type == "add") {
        Utils::assert(instr.size() == 4, "Found bad syntax for " + instr_type + ".");
        BYTE arg1 = Parser::reg_name_to_index(instr[2]);
        BYTE arg2 = Parser::reg_name_to_index(instr[3]);

        rf[arg0] = rf[arg1] + rf[arg2];
        return ret;
    }
    if (instr_type == "and") {
        Utils::assert(instr.size() == 4, "Found bad syntax for " + instr_type + ".");
        BYTE arg1 = Parser::reg_name_to_index(instr[2]);
        BYTE arg2 = Parser::reg_name_to_index(instr[3]);

        rf[arg0] = rf[arg1] & rf[arg2];
        return ret;
    }
    if (instr_type == "slt") {
        Utils::assert(instr.size() == 4, "Found bad syntax for " + instr_type + ".");
        BYTE arg1 = Parser::reg_name_to_index(instr[2]);
        BYTE arg2 = Parser::reg_name_to_index(instr[3]);

        rf[arg0] = (rf[arg1] < rf[arg2] ? 1 : 0);
        return ret;
    }

    // I-type
    if (instr_type == "addi") {
        Utils::assert(instr.size() == 4, "Found bad syntax for " + instr_type + ".");
        int tmp_imm = stoi(instr[3]);
        Utils::assert(-32768 <= tmp_imm && tmp_imm <= 32767, "Found out-of-range imm value for instr " + instr_type + " with imm value " + to_string(tmp_imm) + ".");
        BYTE arg1 = Parser::reg_name_to_index(instr[2]);

        rf[arg0] = rf[arg1] + tmp_imm;
        return ret;
    }
    if (instr_type == "ori") {
        Utils::assert(instr.size() == 4, "Found bad syntax for " + instr_type + ".");
        int tmp_imm = stoi(instr[3]);
        Utils::assert(0 <= tmp_imm && tmp_imm <= 65535, "Found out-of-range imm value for instr " + instr_type + " with imm value " + to_string(tmp_imm) + ".");
        short imm = short(tmp_imm);
        WORD zero_extended_imm = static_cast<unsigned int>(static_cast<unsigned short>(imm));
        BYTE arg1 = Parser::reg_name_to_index(instr[2]);

        rf[arg0] = (rf[arg1] | zero_extended_imm);
        return ret;
    }
    if (instr_type == "lui") {
        Utils::assert(instr.size() == 3, "Found bad syntax for " + instr_type + ".");
        int tmp_imm = stoi(instr[2]);
        Utils::assert(0 <= tmp_imm && tmp_imm <= 65535, "Found out-of-range imm value for instr " + instr_type + " with imm value " + to_string(tmp_imm) + ".");
        unsigned short imm = (unsigned short)tmp_imm;

        rf[arg0] = imm << 16;
        return ret;
    }

    // Memory instructions
    if (instr_type == "lw") {
        Utils::assert(instr.size() == 3, "Found bad syntax for " + instr_type + ".");
        WORD ref_begin = instr[2].find('(');
        WORD ref_end = instr[2].find(')');
        Utils::assert(ref_begin != string::npos && ref_end != string::npos, "Bad memory instruction for instr " + instr_type + ".");
        int tmp_imm = stoi(instr[2].substr(0, ref_begin));
        Utils::assert(-32768 <= tmp_imm && tmp_imm <= 32767, "Found out-of-range imm value for instr " + instr_type + " with imm value " + to_string(tmp_imm) + ".");
        BYTE arg1 = Parser::reg_name_to_index( instr[2].substr(ref_begin + 1, ref_end - ref_begin - 1) );

        WORD loc = rf[arg1] + tmp_imm;
        if (!d_mem.count(loc)) d_mem[loc] = 0;

        rf[arg0] = d_mem[ loc ];
        return ret;
    }
    if (instr_type == "sw") {
        Utils::assert(instr.size() == 3, "Found bad syntax for " + instr_type + ".");
        WORD ref_begin = instr[2].find('(');
        WORD ref_end = instr[2].find(')');
        Utils::assert(ref_begin != string::npos && ref_end != string::npos, "Bad memory instruction for instr " + instr_type + ".");
        int tmp_imm = stoi(instr[2].substr(0, ref_begin));
        Utils::assert(-32768 <= tmp_imm && tmp_imm <= 32767, "Found out-of-range imm value for instr " + instr_type + " with imm value " + to_string(tmp_imm) + ".");
        BYTE arg1 = Parser::reg_name_to_index( instr[2].substr(ref_begin + 1, ref_end - ref_begin - 1) );

        d_mem[ rf[arg1] + tmp_imm ] = rf[arg0];
        return ret;
    }

    // Branch instr
    if (instr_type == "beq") {
        Utils::assert(instr.size() == 4, "Found bad syntax for " + instr_type + ".");
        Utils::assert(labels.count(instr[3]), "Couldn't find label " + instr[3] + ".");
        BYTE arg1 = Parser::reg_name_to_index(instr[2]);

        if (rf[arg0] == rf[arg1]) {
            ret.took_branch = true;
            pc = labels[instr[3]] - 1;
        }

        ret.is_branch = true;
        return ret;
    }

    if (instr_type == "bne") {
        Utils::assert(instr.size() == 4, "Found bad syntax for " + instr_type + ".");
        Utils::assert(labels.count(instr[3]), "Couldn't find label " + instr[3] + ".");
        BYTE arg1 = Parser::reg_name_to_index(instr[2]);

        if (rf[arg0] != rf[arg1]) {
            ret.took_branch = true;
            pc = labels[instr[3]] - 1;
        }

        ret.is_branch = true;
        return ret;
    }

    Utils::error("Somehow reached of instruction parsing without termination. Bad input?\nError instruction: " + instr_type + " " + to_string(int(arg0)) + " ...");

    return ret;
}

bool Parser::valid_instruction(const string& instr) {
    // giant return statement - since its only like 9 instructions idrc it is readable, fast, and inlineable (?) and that is what matters
    // plus the alternatives are for loop or using a hash set, both less readable
    return
    (instr == "add") ||
    (instr == "and") ||
    (instr == "addi") ||
    (instr == "lui") ||
    (instr == "ori") ||
    (instr == "slt") ||
    (instr == "lw") ||
    (instr == "sw") ||
    (instr == "beq") ||
    (instr == "li") ||
    (instr == "j") ||
    (instr == "syscall") ||
    (instr == "bne") ||
    (instr == "move") ||
    (instr == "nop");
}

BYTE Parser::reg_name_to_index(const string& reg) {
    // Basically just a switch case, but not a thing in C++ so this is how it is
    // could replace with a static unordered_map, but who cares

    if (reg == "$zero" || reg == "$0") return 0;
    if (reg == "$at" || reg == "$1") return 1;
    if (reg == "$v0" || reg == "$2") return 2;
    if (reg == "$v1" || reg == "$3") return 3;
    if (reg == "$a0" || reg == "$4") return 4;
    if (reg == "$a1" || reg == "$5") return 5;
    if (reg == "$a2" || reg == "$6") return 6;
    if (reg == "$a3" || reg == "$7") return 7;
    if (reg == "$t0" || reg == "$8") return 8;
    if (reg == "$t1" || reg == "$9") return 9;
    if (reg == "$t2" || reg == "$10") return 10;
    if (reg == "$t3" || reg == "$11") return 11;
    if (reg == "$t4" || reg == "$12") return 12;
    if (reg == "$t5" || reg == "$13") return 13;
    if (reg == "$t6" || reg == "$14") return 14;
    if (reg == "$t7" || reg == "$15") return 15;
    if (reg == "$s0" || reg == "$16") return 16;
    if (reg == "$s1" || reg == "$17") return 17;
    if (reg == "$s2" || reg == "$18") return 18;
    if (reg == "$s3" || reg == "$19") return 19;
    if (reg == "$s4" || reg == "$20") return 20;
    if (reg == "$s5" || reg == "$21") return 21;
    if (reg == "$s6" || reg == "$22") return 22;
    if (reg == "$s7" || reg == "$23") return 23;
    if (reg == "$t8" || reg == "$24") return 24;
    if (reg == "$t9" || reg == "$25") return 25;
    if (reg == "$k0" || reg == "$26") return 26;
    if (reg == "$k1" || reg == "$27") return 27;
    if (reg == "$gp" || reg == "$28") return 28;
    if (reg == "$sp" || reg == "$29") return 29;
    if (reg == "$fp" || reg == "$30") return 30;
    if (reg == "$ra" || reg == "$31") return 31;

    Utils::error("Bad register: " + reg);

    return -1;
}

// Not needed for this lab
WORD Parser::instr_to_hex(token_string& instr, const unordered_map<string, WORD>& label_table, const WORD& tmp_pc) {
    if (instr[0] == "nop") return 0;

    BYTE opcode = 0, rd = 0, rs = 0, rt = 0, funct = 0;
    short imm = 0;

    if (instr[0] == "addi") opcode = 0x8;
    if (instr[0] == "lui") opcode = 0xf;
    if (instr[0] == "ori") opcode = 0xd;
    if (instr[0] == "lw") opcode = 0x23;
    if (instr[0] == "sw") opcode = 0x2b;
    if (instr[0] == "beq") opcode = 0x4;

    if (instr[0] == "add") funct = 0x20;
    if (instr[0] == "and") funct = 0x24;
    if (instr[0] == "slt") funct = 0x2a;

    if (opcode == 0x23 || opcode == 0x2b ) {
        Utils::assert(instr[1].back() == ',', "Missing comma:\n" + instr[0] + " " + instr[1]);
        instr[1].pop_back();

        rt = reg_name_to_index(instr[1]);
        WORD end_of_imm = instr[2].find_first_of('(');
        WORD end_of_string = instr[2].find_first_of(')', end_of_imm);

        int immediate_val = stoi( instr[2].substr(0, end_of_imm) );
        Utils::assert(immediate_val <= 32767 && immediate_val >= -32768, "immediate value out of range [-32768, 32767]:\n" + immediate_val);
        imm = static_cast<short>(immediate_val);

        rs = reg_name_to_index( instr[2].substr(end_of_imm + 1, end_of_string - end_of_imm - 1) );
        return build_i_type(opcode, rs, rt, imm);
    }

    if (opcode == 0xf) {
        Utils::assert(instr[1].back() == ',', "Missing comma:\n" + instr[0] + " " + instr[1]);
        instr[1].pop_back();
        rt = reg_name_to_index(instr[1]);

        int immediate_val = stoi( instr[2] );
        Utils::assert(immediate_val <= 32767 && immediate_val >= -32768, "BEQ immediate value out of range (your program too big) [-32768, 32767]:\n" + instr[3] + " => " + to_string(immediate_val));
        imm = static_cast<short>(immediate_val);
        return build_i_type(opcode, rs, rt, imm);
    }

    if (!opcode) {
        Utils::assert(instr[1].back() == ',', "Missing comma:\n" + instr[0] + " " + instr[1]);
        instr[1].pop_back();
        Utils::assert(instr[2].back() == ',', "Missing comma:\n" + instr[0] + " " + instr[1] + " " + instr[2]);
        instr[2].pop_back();

        rd = reg_name_to_index(instr[1]);
        rs = reg_name_to_index(instr[2]);
        rt = reg_name_to_index(instr[3]);

        return build_r_type(rd, rs, rt, funct);
    }

    Utils::assert(instr[1].back() == ',', "Missing comma:\n" + instr[0] + " " + instr[1]);
    instr[1].pop_back();
    Utils::assert(instr[2].back() == ',', "Missing comma:\n" + instr[0] + " " + instr[1] + " " + instr[2]);
    instr[2].pop_back();

    rt = reg_name_to_index(instr[1]);
    rs = reg_name_to_index(instr[2]);

    int immediate_val = 0;
    if (opcode == 4) immediate_val = label_table.at(instr[3]) - tmp_pc - 1;
    else immediate_val = stoi( instr[3] );

    Utils::assert(immediate_val <= 32767 && immediate_val >= -32768, "BEQ immediate value out of range (your program too big) [-32768, 32767]:\n" + instr[3] + " => " + to_string(immediate_val));
    imm = static_cast<short>(immediate_val);
    
    return build_i_type(opcode, rs, rt, imm);
}

WORD Parser::build_i_type(const BYTE& opcode, const BYTE& rs, const BYTE& rt, const short& imm) {
    WORD out = opcode;
    out <<= 5;
    out |= rs;
    out <<= 5;
    out |= rt;
    out <<= 16;
    out |= static_cast<WORD>(imm) & 0xFFFF;

    return out;
}

WORD Parser::build_r_type(const BYTE& rd, const BYTE& rs, const BYTE& rt, const BYTE& funct) {
    WORD out = rs;
    out <<= 5;
    out |= rt;
    out <<= 5;
    out |= rd;
    out <<= 11;
    out |= funct;

    return out;
}

string Parser::get_first_token(const string& line) {
    if (line.empty()) return "";
    WORD start = line.find_first_not_of(" \t\n\r");
    if (start == WORD(string::npos)) return "";
    WORD end = line.find_first_of(" \t\n\r", start);
    
    return line.substr(start, end - start);
}

Parser::token_string Parser::split(const string& line) {
    istringstream splitter(line);
    token_string out;
    for (string token; splitter >> token;) {
        if (token.front() == '#') break;            // Break entirely if comment
        if (token.back() == ',') token.pop_back();  // Remove commas
        out.push_back(token);
    }
    return out;
}

void Utils::assert(const bool& good_eval, const string& on_error) {
    if (good_eval) return;
    Utils::error(on_error);
}

void Utils::error(const string& exit_msg) {
    cerr << "Encountered error:\n" << exit_msg << endl;
    exit(1);
}

#endif