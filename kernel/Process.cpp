#include "Process.h"
#include "kstl/Array.h"

extern "C" void run_process(kernel::TrapFrame& trapFrame);

kernel::PCB::PCB(const char* binaryFile) : PID(1), state(READY) {
    VMPackage fpkg(binaryFile, O_RDONLY, FREAD);
    
    ministl::array<char, 128> buffer;
    fpkg.args.fread.buffer = (uint32_t)buffer.data();
    fpkg.args.fread.nbytes = buffer.capacity();
    
    // needs to read more than one 128 read obv in future
    uint32_t bytesRead = fpkg.send().res;

    // init non-zero regs
    trapFrame.epc = 0x00400020; // will implicitly gain a +4
    trapFrame.sp  = 0x7ffffffc;
    trapFrame.gp  = 0x10008000;

    // write the binary into memory, will have to replace this with ELF handling later on
    char* placeFile = (char*)(trapFrame.epc + 4);
    for (uint32_t i = 0; i < bytesRead; ++i) placeFile[i] = buffer[i];
}

void kernel::PCB::run() {

    run_process(trapFrame);
}
