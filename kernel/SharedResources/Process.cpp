#include "Manager.h"
#include "../kstl/String.h"
#include "../kstl/File.h"
#include "../kstl/Elf.h"
#include "../KernelObjects/PageTables.h"

kernel::PCB::Guard::Guard() : ref(nullptr) {}
kernel::PCB::Guard::Guard(PCB& pcb) : ref(&pcb) {}

void kernel::PCB::Guard::reset() {
    PCB* pcb = ref;
    ref = nullptr;
    if (!pcb || pcb->PID == ProcessManager::KERNEL_PID ) return;
    --pcb->refcount;
    if ( pcb->state == ZOMBIE && pcb->refcount == 0 ) {
        sharedResources.processes.freeProcess( pcb->getPID() );
    }
}

kernel::PCB::Guard::Guard(Guard&& other) {
    ref = other.ref;
    other.ref = nullptr;
}
kernel::PCB::Guard& kernel::PCB::Guard::operator=(Guard&& other) {
    if (this == &other) return *this;
    reset();
    ref = other.ref;
    other.ref = nullptr;
    return *this;
}
kernel::PCB::Guard::~Guard() {
    reset();
}

kernel::PCB::Guard kernel::PCB::borrow() {

    ++refcount;
    return Guard( *this );
}

void kernel::PCB::Guard::setAsCurrentThread() {
    assert(ref);
    currentThread = ref;
}

void kernel::PCB::markForDeath() {
    assert( PID != ProcessManager::KERNEL_PID && "The kernel process should never be marked for death" );
    state = ZOMBIE;
}

bool kernel::PCB::exec(const char* executableFile, RegisterContext* trapCtx ) {
    assert( PID != ProcessManager::KERNEL_PID );

    // For simplicity/Code reuse sake, we'll just make a new process and take it's page table, since that's the only expensive resource
    // Maybe later make some functions that are dedicated to make page tables?

    uint32_t newPID = sharedResources.processes.createProcess( executableFile );
    if (newPID == NOPCBEXISTS) return false;

    PCB::Guard newProc = sharedResources.processes[newPID];
    addrSpace._pageTable = ministl::move( newProc->addrSpace._pageTable );
    if (trapCtx) *trapCtx = newProc->regCtx;
    else regCtx = newProc->regCtx;
    newProc->markForDeath();

    return true;
}


kernel::PCB::PCB() : regCtx{}, addrSpace( ministl::unique_ptr<PageTable>() ), state(READY), priority(0), refcount(0), PID(0) {}

kernel::PCB::PCB(uint32_t pid, ProcessState state, ministl::unique_ptr<PageTable> pageSystem)
    : regCtx{}, addrSpace( ministl::move(pageSystem) ), state(state), priority(0), refcount(0), PID(pid) {}

kernel::PCB::PCB(kernelInit_t, KernelPageTable& kpt) 
: regCtx{}, addrSpace(kernelInitalizer, kpt), state(RUNNING), priority(0), refcount(0), PID(ProcessManager::KERNEL_PID) {
    
    currentThread = this;
}

kernel::ProcessManager::ProcessManager() : kPageTable(), kernelProcess(kernelInitalizer, kPageTable), processes(), freePids() {}

kernel::PCB::Guard kernel::ProcessManager::operator[](size_t idx) {
    if (idx >= processes.size() || !processes[idx] ) return PCB::Guard();
    return processes[idx]->borrow();
}


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

#define MIN(x, y) (x < y) ? x : y

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

uint32_t kernel::ProcessManager::createProcess(const char* executableFile) {

    // ELF parsing avoids heap-allocation and instead uses buffers since heap-allocation could get really expensive memory-wise
    // Worth more than the speed-tradeoff, which is still arguable from vector resizing

    kernel::File file(executableFile, O_RDONLY);

    // ELF header
    Elf32_Ehdr ehdr{};
    if (!read_full(file, &ehdr, sizeof(ehdr))) {
        return NOPCBEXISTS;
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
        uint32_t bss_addr, bss_size;
    } info{};
    info.entry = ehdr.e_entry;

    //  Read the section-header for the shstrtab itself (single record)
    Elf32_Shdr shstr_sh{};
    file.seek(shoff + static_cast<uint32_t>(shstrndx) * shentsize, 0);
    if (!read_full(file, &shstr_sh, sizeof(shstr_sh))) {
        return NOPCBEXISTS;
    }

    // Scan all section headers one-by-one, resolve names on-demand
    char namebuf[128]; // adjust if section names can exceed this
    for (uint16_t i = 0; i < shnum; ++i) {
        Elf32_Shdr sh{};
        file.seek(shoff + static_cast<uint32_t>(i) * shentsize, 0);
        if (!read_full(file, &sh, sizeof(sh))) { return NOPCBEXISTS; }

        if (!read_cstr_from_shstr(file, shstr_sh.sh_offset, sh.sh_name, namebuf, sizeof(namebuf))) {
            return NOPCBEXISTS;
        }

        if (ministl::streq(namebuf, ".text")) {
            info.text_offset = sh.sh_offset;
            info.text_size   = sh.sh_size;
        } else if (ministl::streq(namebuf, ".data")) {
            info.data_offset = sh.sh_offset;
            info.data_size   = sh.sh_size;
        } else if (ministl::streq(namebuf, ".bss")) {
            info.bss_addr = sh.sh_addr;
            info.bss_size = sh.sh_size;
        }
    }

    // Allocate PCB now that we know the file should be good and with stack/data awareness
    assert(info.data_size < 8_kb && "Static section can only be 8kb in size according to mips calling card, consider changing since that's not a lot");

    size_t newPID;
    if ( !freePids.empty() ) {
        newPID = freePids.back(); // Don't pop yet just in case there's a failure
    } else {
        newPID = processes.size();
    }

    // Need to round up on the pages alloced, for example if size is only 400 bytes, 400 >> 12 == 0.
    // can't use make_unique since it's private to other classes, have to alloc in place
    // TODO: Use a PageTable factory instead?
    ministl::unique_ptr<PCB> newProcess( 
        new PCB(
            newPID, 
            BLOCKED, 
            ministl::make_unique<HashPageTable>((info.text_size >> 12) + 1, ( (info.data_size + info.bss_size) >> 12) + 1, 4)
        ) 
    ); 

    // Don't add to vector yet just in case of failure
    
    // Now we "establish" the process, registers and write in text/static
    newProcess->regCtx.accessRegister(kernel::SP) = kernel::STACK_LIMIT;
    newProcess->regCtx.accessRegister(kernel::GP) = kernel::DYNAMIC_START;

    if (info.text_size) {
        file.seek(info.text_offset, 0);

        // Write directly to the corresponding physical pages using the PFN in the PTEs stored in the AddrSpace of the proc
        // We can avoid the tblwriting overhead

        size_t bytesLeft = info.text_size;
        char* dst = (char*)(kernel::TEXT_START);

        do {
            char* physAddr = (char*)(kernel::STACK_LIMIT) + ( (newProcess->addrSpace.translate( (uint32_t)dst ).lo & ~uint32_t(0b111111)) << 6 );
            size_t readAmt = MIN( bytesLeft, kernel::PAGE_SIZE );
            if (!stream_copy_to(file, (void*)physAddr, readAmt )) { return NOPCBEXISTS; }
            bytesLeft -= readAmt;
            dst += readAmt;
        } while ( bytesLeft > 0 );
    }

    if (info.data_size) {
        file.seek(info.data_offset, 0);
        
        size_t bytesLeft = info.data_size;
        char* dst = (char*)(kernel::STATIC_START);

        do {
            char* physAddr = (char*)(kernel::STACK_LIMIT) + ( (newProcess->addrSpace.translate( (uint32_t)dst ).lo & ~uint32_t(0b111111)) << 6 );
            size_t readAmt = MIN( bytesLeft, kernel::PAGE_SIZE );
            if (!stream_copy_to(file, (void*)physAddr, readAmt )) { return NOPCBEXISTS; }
            bytesLeft -= readAmt;
            dst += readAmt;
        } while ( bytesLeft > 0 );
    }

    // Right now, the startup function zeroes the bss. We'll have the OS do this later

    newProcess->regCtx.epc = info.entry - 4;

    // Since everything has gone swimmingly, take resources

    if ( !freePids.empty() && newPID == freePids.back() ) { // This is not gonna work when multi-threaded, but we'll get there when we get there
        freePids.pop_back(); 
        auto& procSlot = processes[newPID];
        assert(!procSlot);
        procSlot = ministl::move(newProcess);
        return newPID;
    }

    processes.emplace_back( ministl::move(newProcess) );

    return newPID;
}

uint32_t kernel::ProcessManager::forkProcess(uint32_t pid, RegisterContext* newProcRegContext ) {
    if ( pid >= processes.size() || !processes[pid] ) return NOPCBEXISTS;
    auto& copyProcess = processes[pid];

    ministl::unique_ptr<PCB> newProcess( new PCB(
        0,
        READY,
        ministl::make_unique<HashPageTable>( copyProcess->addrSpace._pageTable->getIterator() ) // Implicitly copies the page's underlying memory as well
    ));

    if (newProcRegContext) newProcess->regCtx = *newProcRegContext;

    uint32_t ret;
    if ( freePids.empty() ) {
        newProcess->PID = processes.size();
        ret = newProcess->PID;
        processes.emplace_back( ministl::move(newProcess) );        
    } else {
        newProcess->PID = freePids.back();
        freePids.pop_back();
        ret = newProcess->PID;
        assert( !processes[newProcess->PID] );
        processes[newProcess->PID] = ministl::move(newProcess);
    }

    return ret;
}

void kernel::ProcessManager::freeProcess( uint32_t pid ) {
    assert( pid < processes.size() );
    auto& procContainer = processes[pid];
    assert( procContainer && procContainer->refcount == 0 && procContainer->state == ZOMBIE ); 
    // All the conditions for a valid free - the PCB exists, there are no remaining references, and it is zombie

    if ( pid == processes.size() - 1 ) {
        processes.pop_back();
        return;
    }

    procContainer.reset();
    freePids.push_back(pid);

}