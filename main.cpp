#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include "machine/Hardware.h"
#include "machine/BinaryUtils.h"
#include "loader/Loader.h"

// -- Dynamic debug loading --
#ifdef DEBUG
    #include "debug/debughooks.cpp"
    #define DBGHOOK(hook) hook
#else
    #define DBGHOOK(hook) nullptr
#endif

using namespace std;

int main(int argc, char** argv) {

    // -- Init tasks --
    if (argc < 2) {
        std::cout << "Need at least one program to run.\n";
        return 1;
    }
    ios_base::sync_with_stdio(false);

    std::vector<std::string> kernelArgs(argc - 1);
    for (int i = 1; i < argc; ++i) kernelArgs.push_back(argv[i]);

    // -- Setup debugging if enabled --
    #ifdef DEBUG
        std::ofstream out("dbgout.tmp", std::ios::out | std::ios::trunc);
        dbg_output_stream = &out;
    #endif

    // -- Load Kernel --
    FileLoader::KernelLoader kernel("kernel/kernel.elf");
    if (kernel.bad()) {
        cout << "Failed to load kernel." << endl;
        return 1;
    }

    // -- Setup and run machine --
    Hardware::Machine machine;
    machine.loadKernel( kernel.kernelInfo, kernelArgs);
    machine.run(DBGHOOK((
        makeCombinedHook<printRegs, printInstr>()
    )));

    return 0;
}