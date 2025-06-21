#include "File.h"
#include "Array.h"

kernel::File::File(const char* fileName, uint32_t flags) : fileIO(FOPEN) {
    bad = false;
    fileIO.args.fopen.path  = (uint32_t)fileName;
    fileIO.args.fopen.flags = flags;
    VMResponse response = fileIO.send();

    bad = bool(response.err);

    fileIO.args.generic.arg0 = response.res;
}

kernel::File::~File() {
    fileIO.reqType = FCLOSE;
    fileIO.send();
}

ministl::vector<char> kernel::File::read(uint32_t bytes) {
    fileIO.reqType = FREAD;
    ministl::array<char, 128> buffer;
    ministl::vector<char> out;
    out.reserve(257);

    uint32_t error = 0;

    while (error == 0 && bytes > 0) {
        uint32_t chunk = (bytes < 128) ? bytes : 128;
        fileIO.args.fread.buffer = (uint32_t)(buffer.data());
        fileIO.args.fread.nbytes = chunk;

        VMResponse response = fileIO.send();
        error = response.err;

        for (uint32_t i = 0; i < response.res; ++i) out.push_back(buffer[i]);
        bytes -= response.res;

        if (response.res == 0) break;  // Prevent infinite loop on partial reads
    }

    return out;
}

kernel::uint32_t kernel::File::seek(uint32_t offset, int whence) {
    fileIO.reqType = FSEEK;
    fileIO.args.fseek.offset = offset;
    fileIO.args.fseek.whence = whence;
    VMResponse response = fileIO.send();

    if (response.err) bad = true;

    return response.res;
}