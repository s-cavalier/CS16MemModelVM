#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include "Hardware.h"
#include "Instruction.h"
#include "BinaryInstruction.h"

using namespace std;

template <typename T>
using SafeVector = vector<unique_ptr<T>>;

int main(int argc, char** argv) {
    ios_base::sync_with_stdio(false);
    ifstream executable(argv[1], ios::binary);
    if (executable.bad()) exit(1);
 
    std::vector<Word> instructions;

    Byte buffer[4];
    while (executable.read((char*)buffer, 4)) {
        Word instruction = loadBigEndian(buffer);
        instructions.push_back(instruction);
    }

    cout << "Loaded instructions:\n";
    for (const auto & instr: instructions) cout << hex << "Instruction: " << instr << '\n';
    cout << '\n';

    Hardware::Machine machine;
    machine.loadInstructions(instructions);

    while (!machine.killProcess()) {
        machine.runInstruction();
    }

    cout << "REGISTERS:\n";
    for (int i = 0; i < 32; ++i) cout << '$' << i << " = " << machine.readRegister(i) << '\n';
    cout << "MEMORY:\n";
    for (const auto& kv : machine.readMemory()) {
        //if (kv.first <= 0x10000000) continue;
        cout << hex << kv.first << " = 0x" << setw(2) << setfill('0') << Word(kv.second) << '\n';
    }

    return 0;
}