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

kernel::uint32_t kernel::File::read(char* buffer, uint32_t bytes) {
    if (!buffer || bytes == 0) return 0;

    fileIO.reqType           = FREAD;
    fileIO.args.fread.buffer = (uint32_t)buffer;

    const uint32_t maxChunkSize = 256;
    fileIO.args.fread.nbytes = (bytes < maxChunkSize) ? bytes : maxChunkSize;

    VMResponse response = fileIO.send();
    if (response.err) {
        bad = true;
        return 0;
    }

    return response.res;
}



kernel::uint32_t kernel::File::seek(uint32_t offset, int whence) {
    fileIO.reqType = FSEEK;
    fileIO.args.fseek.offset = offset;
    fileIO.args.fseek.whence = whence;
    VMResponse response = fileIO.send();

    if (response.err) bad = true;

    return response.res;
}