#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unistd.h>
#include <sys/wait.h>
#include "Utils.h"

using namespace std;
using namespace Utils; // Only use namespace utils, since these should be global here but exist in namespace in Utils.h for cleaner codespace

int main(int argc, char** argv) {

    // Check for valid input
    assert(argc == 3 || argc == 4, "USAGE: ./mipsinterpret <MIPS src> <trace output> [-pf (print final memory)]");
    bool PRINT_FINAL = false;

    if (argc == 4) {
        assert(string(argv[3]) == "-pf", "USAGE: ./mipsinterpret <MIPS src> <trace output> [-pf (print final memory)]");
        PRINT_FINAL = true;
    }

    // First is to compile into both internal memory and pyrtl CPU mem

    ifstream MIPS_src(argv[1]);
    assert(MIPS_src.good(), "Failed to open file " + string(argv[1]) + ".");

    ofstream output_file(argv[2]);
    assert(output_file.good(), "Failed to open file " + string(argv[2]) + ".");

    // First pass, "load into text section" (i.e., cpu.labels and cpu.i_mem)

    Hardware::CPU cpu;                                          // Main computer
    vector<string> label_record;                                // Stores labels with no matched instruction in case they get stacked
    WORD line_counter = 0;                                      // Used for debugging
    
    for (string line; getline(MIPS_src, line);) {
        ++line_counter;

        Parser::token_string token_string = Parser::split(line);    // Split into tokens, break if comment
        if (token_string.empty()) continue;
        
        // Get first token, check for basic errors
        string& first_token = token_string[0];
        if (first_token.front() == '.') assert(first_token == ".text" || first_token == ".globl", "Found an unrecognized directive at " + to_string(line_counter) + ". Found:\n" + first_token);
        else assert(Parser::valid_instruction(first_token) || first_token.back() == ':', "Unrecognized syntax at line " + to_string(line_counter) + ". Found:\n" + first_token);

        if (first_token == ".globl") {
            cout << "Warning: Seeing globl directive at line " + to_string(line_counter) + ". Ignoring and entering at main...\n";
            continue;
        }

        if (first_token == ".text") continue;


        // If label, get it ready for the next instruction
        if (first_token.back() == ':') {
            first_token.pop_back();
            assert(!cpu.labels.count(first_token), "Seeing repeat label " + first_token + " at line " + to_string(line_counter) + ". Don't know how to resolve.");

            label_record.push_back(first_token);
            continue;
        }
        
        // Dump labels onto the newfound instruction
        while (!label_record.empty()) {
            cpu.labels[label_record.back()] = cpu.pc;
            label_record.pop_back();
        }

        // Add instruction to i_mem
        cpu.i_mem.push_back(token_string);

        ++cpu.pc;
    }

    cpu.END_OF_PROGRAM = cpu.pc;

    // Dump empty labels with the last pc (end of program)
    while (!label_record.empty()) {
        cpu.labels[label_record.back()] = cpu.pc;
        label_record.pop_back();
    }

    assert(cpu.labels.count("main"), "No main label found.");

    // Reset PC and run program!
    cpu.pc = cpu.labels["main"];

    // initialize stack pointer too lol
    cpu.rf[29] = 0x7FFFFFFC;

    for (; cpu.pc < cpu.END_OF_PROGRAM; ++cpu.pc) {
        Hardware::lab6_branch_info branch_info = cpu.run_instruction();
        output_file << branch_info.unpack() << '\n';
    }

    if (PRINT_FINAL) {
        cout << "COMMA-SEPERATED REGISTER FILE:" << endl;
        for (int i = 0; i < 32; ++i) cout << '$' << i << '=' << cpu.rf[i] << ',';
        cout << endl;
        cout << "COMMA-SEPERATED USED RAM:" << endl;
        for (const auto &kv : cpu.d_mem) cout << kv.first << '=' << kv.second << ',';
    }

    return 0;
}