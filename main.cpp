#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include "Hardware.h"
#include "Instruction.h"
#include "BinaryUtils.h"

// FUTURE: watch out for delay slots?

using namespace std;

template <typename T>
using SafeVector = vector<unique_ptr<T>>;

void loadFromExecutable(const string& path, vector<Word>& storage) {
    ifstream executable(path, ios::binary);
    if (executable.bad()) return;

    Byte buffer[4];
    while (executable.read((char*)buffer, 4)) storage.push_back(Binary::loadBigEndian(buffer));
}

int main(int argc, char** argv) {
    ios_base::sync_with_stdio(false);
 
    std::vector<Word> instructions;
    loadFromExecutable(argv[1], instructions);

    instructions[0] |= stoul(argv[2]);

    Hardware::Machine machine;
    machine.loadInstructions(instructions);
    machine.run();

    // cout << "REGISTERS:\n";
    // for (int i = 0; i < 32; ++i) cout << '$' << Binary::regToString[i] << " = " << hex << machine.readRegister(i) << '\n';
    // cout << "MEMORY:\n";
    // for (const auto& kv : machine.readMemory()) {
    //     if (kv.first <= 0x10000000) continue;
    //     cout << hex << kv.first << " = 0x" << setw(2) << setfill('0') << Word(kv.second) << '\n';
    // }

    return 0;
}