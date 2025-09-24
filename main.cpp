#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <iomanip>
#include "machine/Hardware.h"
#include "machine/BinaryUtils.h"
#include "loader/Loader.h"
#include "network/Server.h"

int main(int argc, char** argv) {

    std::ios_base::sync_with_stdio(false);

    // FileLoader::KernelLoader kernel("kernel/kernel.elf");

    // assert( !kernel.bad() );

    // // -- Setup and run machine --
    // Hardware::Machine machine;
    // machine.loadKernel( kernel.kernelInfo, { "programs/shell/shell.elf" } );
    // machine.run();


    Server server;

    server.listen();

    
    return 0;
}