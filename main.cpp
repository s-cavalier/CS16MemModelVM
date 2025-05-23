#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cstring>
#include "machine/Hardware.h"
#include "machine/BinaryUtils.h"
#include "machine/Loader.h"

// FUTURE: watch out for delay slots?

using namespace std;

template <typename T>
using SafeVector = vector<unique_ptr<T>>;

int main(int argc, char** argv) {
    if (argc != 3) return 1;

    ios_base::sync_with_stdio(false);
 
    FileLoader::ExecutableParser exe(argv[1]);
    if (exe.bad()) {
        cout << "Failed to load file '" << argv[1] << '\'' << endl;
        return 1;
    }

    float input = stof(argv[2]);
    Word arg = 0;
    memcpy(&arg, &input, 4);
    HalfWord upper = arg >> 16;
    HalfWord lower = arg & 0xFFFF;

    exe.readText()[0] &= 0xFFFF0000;
    exe.readText()[0] |= upper;
    exe.readText()[1] &= 0xFFFF0000;
    exe.readText()[1] |= lower;
    
    Hardware::Machine machine;
    machine.loadInstructions(exe.readText());
    machine.run();

   cout << machine.readFPRegister(4) << endl;

    return 0;
}