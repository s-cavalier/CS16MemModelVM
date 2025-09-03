#include "Process.h"
#include "kstl/String.h"
#include "kstl/File.h"
#include "kstl/Elf.h"

extern "C" char _end[];

kernel::MemoryManager::MemoryManager() {
    kernelReservedBoundary = ((size_t(_end) - 0x80000000) + PAGE_SIZE - 1) / PAGE_SIZE; // round up to closet page boundary in kernel image
}

size_t kernel::MemoryManager::reserveFreeFrame() {
    size_t frame = size_t(-1);
    for (size_t i = kernelReservedBoundary; i < freePages.size(); ++i) {
        if (freePages[i]) continue; // reserved if bit[i] == 1
        frame = i;
        break;
    }

    assert(frame != size_t(-1)); // ran out of pages
}

kernel::PCB::PCB() : PID(0), state(RUNNING) {}


// ----- internal helpers
static bool read_full(kernel::File& f, void* dst, size_t n) {
    char* p = static_cast<char*>(dst);
    size_t remaining = n;
    while (remaining > 0) {
        uint32_t got = f.read(p, remaining);
        if (got == 0) return false; // EOF early or error
        p         += got;
        remaining -= got;
    }
    return true;
}

static bool stream_copy_to(kernel::File& f, void* dst, size_t n) {
    char* out = static_cast<char*>(dst);
    char buf[512]; // stack buffer for I/O
    size_t remaining = n;
    while (remaining > 0) {
        size_t want = (remaining < sizeof(buf)) ? remaining : sizeof(buf);
        uint32_t got = f.read(buf, want);
        if (got == 0) return false; // EOF early or error
        for (uint32_t i = 0; i < got; ++i) out[i] = buf[i];
        out       += got;
        remaining -= got;
    }
    return true;
}

static bool read_cstr_from_shstr(kernel::File& f, uint32_t shstr_off, uint32_t name_off, char* scratch, size_t scratch_sz) {
    f.seek(shstr_off + name_off, 0);
    size_t pos = 0;
    while (pos + 1 < scratch_sz) {
        uint32_t got = f.read(&scratch[pos], 1);
        if (got == 0) return false;     // unexpected EOF
        if (scratch[pos] == '\0') return true;
        ++pos;
    }
    if (scratch_sz) scratch[scratch_sz - 1] = '\0'; // ensure null if truncated
    return true;
}

// ---- constructor (no dynamic allocation) 
kernel::PCB::PCB(const char* binaryFile, bool fromSpim) : PID(1), state(READY) {
    // init non-zero regs
    regCtx.accessRegister(kernel::SP) = 0x7ffffffc;
    regCtx.accessRegister(kernel::GP) = 0x10008000;

    if (fromSpim) {
        regCtx.epc = 0x00400020; // will implicitly gain a +4
        char* placeFile = reinterpret_cast<char*>(regCtx.epc + 4);

        kernel::File file(binaryFile, O_RDONLY);

        char buf[256];
        for (;;) {
            uint32_t got = file.read(buf, sizeof(buf));
            if (got == 0) break; // EOF
            for (uint32_t i = 0; i < got; ++i) *(placeFile++) = buf[i];
        }
        return;
    }

    // --- ELF path (no heap allocations) ---
    kernel::File file(binaryFile, O_RDONLY);

    // 1) ELF header
    Elf32_Ehdr ehdr{};
    if (!read_full(file, &ehdr, sizeof(ehdr))) {
        state = ZOMBIE; return;
    }

    // Gather core fields
    const uint32_t shoff     = ehdr.e_shoff;
    const uint16_t shnum     = ehdr.e_shnum;
    const uint16_t shentsize = ehdr.e_shentsize;
    const uint16_t shstrndx  = ehdr.e_shstrndx;

    struct {
        uint32_t entry;
        uint32_t text_offset, text_size;
        uint32_t data_offset, data_size;
    } info{};
    info.entry = ehdr.e_entry;

    // 2) Read the section-header for the shstrtab itself (single record)
    Elf32_Shdr shstr_sh{};
    file.seek(shoff + static_cast<uint32_t>(shstrndx) * shentsize, 0);
    if (!read_full(file, &shstr_sh, sizeof(shstr_sh))) {
        state = ZOMBIE; return;
    }

    // 3) Scan all section headers one-by-one, resolve names on-demand
    char namebuf[64]; // adjust if section names can exceed this
    for (uint16_t i = 0; i < shnum; ++i) {
        Elf32_Shdr sh{};
        file.seek(shoff + static_cast<uint32_t>(i) * shentsize, 0);
        if (!read_full(file, &sh, sizeof(sh))) { state = ZOMBIE; return; }

        if (!read_cstr_from_shstr(file, shstr_sh.sh_offset, sh.sh_name,
                                  namebuf, sizeof(namebuf))) {
            state = ZOMBIE; return;
        }

        if (ministl::streq(namebuf, ".text")) {
            info.text_offset = sh.sh_offset;
            info.text_size   = sh.sh_size;
        } else if (ministl::streq(namebuf, ".data")) {
            info.data_offset = sh.sh_offset;
            info.data_size   = sh.sh_size;
        }
    }

    // 4) Stream sections directly into memory with a stack buffer
    if (info.text_size) {
        file.seek(info.text_offset, 0);
        char* textDst = reinterpret_cast<char*>(0x00400024);
        if (!stream_copy_to(file, textDst, info.text_size)) { state = ZOMBIE; return; }
    }

    if (info.data_size) {
        file.seek(info.data_offset, 0);
        char* dataDst = reinterpret_cast<char*>(0x10008000);
        if (!stream_copy_to(file, dataDst, info.data_size)) { state = ZOMBIE; return; }
    }

    regCtx.epc = info.entry - 4;
}
