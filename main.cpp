#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include "machine/Hardware.h"
#include "machine/BinaryUtils.h"
#include "machine/Loader.h"

// FUTURE: watch out for delay slots?

using namespace std;

template <typename T>
using SafeVector = vector<unique_ptr<T>>;

int main(int argc, char** argv) {
    if (argc != 2) return 1;

    ios_base::sync_with_stdio(false);
 
    unique_ptr<FileLoader::Parser> exe = make_unique<FileLoader::ELFLoader>(argv[1]); 
    if (exe->bad()) {
        cout << "Failed to load file '" << argv[1] << '\'' << endl;
        return 1;
    }

    Hardware::Machine machine;
    machine.loadInstructions(exe->readText());
    machine.run();

    cout << "REGISTERS:\n";
    for (int i = 0; i < 32; ++i) cout << '$' << Binary::regToString[i] << " = " << hex << machine.readRegister(i) << '\n';
    cout <<"FP REGISTERs:\n";
    for (int i = 0; i < 32; ++i) cout << "$f" << i << " = " << machine.readFPRegister(i) << '\n'; 
    cout << "MEMORY:\n";
    for (const auto& kv : machine.readMemory()) {
        if (kv.first <= 0x10000000) continue;
        cout << hex << kv.first << " = 0x" << setw(2) << setfill('0') << Word(kv.second) << '\n';
    }

    return 0;
}