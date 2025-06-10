#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include "machine/Hardware.h"
#include "machine/BinaryUtils.h"
#include "loader/Loader.h"

// dynamic debug loading
#ifdef DEBUG
    #include "debug/debughooks.cpp"
    #define DBGHOOK(hook) hook
#else
    #define DBGHOOK(hook) nullptr
#endif

using namespace std;

template <typename T>
using SafeVector = vector<unique_ptr<T>>;

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) return 1;
    ios_base::sync_with_stdio(false);

    #ifdef DEBUG
        std::ofstream out("dbgout.tmp", std::ios::out | std::ios::trunc);
        dbg_output_stream = &out;
    #endif

    unique_ptr<FileLoader::Parser> exe;
    FileLoader::KernelLoader kernel("kernel/kernel.elf");

    if (argc == 3 && string(argv[2]) == "-spim") exe = make_unique<FileLoader::SpimLoader>(argv[1]);
    else exe = make_unique<FileLoader::ELFLoader>(argv[1]);

    if (exe->bad()) {
        cout << "Failed to load file '" << argv[1] << '\'' << endl;
        return 1;
    }

    if (kernel.bad()) {
        cout << "Failed to load kernel." << endl;
        return 1;
    }

    Hardware::Machine machine;

    machine.loadKernel(kernel.readText(), kernel.readData(), kernel.readEntry(), kernel.getTrapHandlerLocation());
    machine.loadProgram(exe->readText(), exe->readData(), exe->readEntry());

    machine.run(DBGHOOK((
        makeConditionalCombinedHook<ignoreKernel, printRegs>()
    )));

    return 0;
}