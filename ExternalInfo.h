#ifndef __EXTERNAL_INFO_H__
#define __EXTERNAL_INFO_H__
#include <vector>

// -------------
// External Info
// -------------
// Space for all boot interface so the loader and machine functionality can work independently.
// For right now, this is just bootloader info. Later, it'll be more complex.
// Will eventually deal with loading some file onto a "diskfile" when filesystem is made
// Will also include a way to bootload programs (probably? maybe don't need if diskfile exsts)

namespace ExternalInfo {
    using Word = unsigned int;
    using Byte = unsigned char;

    struct KernelBootInformation {
        std::vector<Word> text;
        std::vector<Byte> data;
        Word bootEntry;
        Word trapEntry;
        Word argc;
        Word argv;
    };
}


#endif