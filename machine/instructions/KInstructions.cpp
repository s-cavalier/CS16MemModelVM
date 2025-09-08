#include "KInstructions.h"
#include "../BinaryUtils.h"
#include <iostream>
#include <string>

#define EXL_CHECK if (!checkEXL()) throw Hardware::Trap(Hardware::Trap::RI)

KInstruction::KInstruction(K_INSTR_ARGS) :  statusRegister( statusRegister ) {}

MoveToCoprocessor0::MoveToCoprocessor0(K_INSTR_ARGS, const int& rt, int& rd) : 
    KInstruction(statusRegister), rt(rt), rd(rd) {}

void MoveToCoprocessor0::run() { EXL_CHECK; rd = rt; }

MoveFromCoprocessor0::MoveFromCoprocessor0(K_INSTR_ARGS, int& rt, const int& rd) : KInstruction(statusRegister), rt(rt), rd(rd) {}

void MoveFromCoprocessor0::run() { EXL_CHECK; rt = rd; }

ExceptionReturn::ExceptionReturn(Hardware::Machine& machine) : KInstruction(machine.accessCoprocessor(0)->accessRegister(Binary::STATUS).ui ), machine(machine) {}

void ExceptionReturn::run() { 
    EXL_CHECK; 
    machine.accessCPU().accessProgramCounter() = machine.accessCoprocessor(0)->readRegister(Binary::EPC).ui;

}

TLBInstruction::TLBInstruction(TLB_INSTR_ARGS) : KInstruction(statusRegister), tlb(tlb) {}

TLBProbe::TLBProbe(TLB_INSTR_ARGS, Word& indexRegister, Word& entryHiRegister) : TLBInstruction(statusRegister, tlb), indexRegister(indexRegister), entryHiRegister(entryHiRegister) {}
void TLBProbe::run() {
    EXL_CHECK;
    Hardware::TLBEntry fakeEntry(entryHiRegister, 0);
    for (Byte i = 0; i < TLB_ENTRIES; ++i) {
        if ( tlb[i].vpn == fakeEntry.vpn && ( tlb[i].global || (tlb[i].asid == fakeEntry.asid) ) ) {
            indexRegister = i;
            return;
        }

    }

    indexRegister = 0x80000000;
}

TLBReadIndexed::TLBReadIndexed(TLB_INSTR_ARGS, const Word& indexRegister, Word& entryHiRegister, Word& entryLoRegister) : TLBInstruction(statusRegister, tlb),
    indexRegister(indexRegister), entryHiRegister(entryHiRegister), entryLoRegister(entryLoRegister) {}

void TLBReadIndexed::run() {
    EXL_CHECK;

    assert(indexRegister < TLB_ENTRIES);

    auto [hi, lo] = tlb[indexRegister].hiLoPair();
    entryHiRegister = hi;
    entryLoRegister = lo;
}

TLBWriteIndexed::TLBWriteIndexed(TLB_INSTR_ARGS, Word& indexRegister, Word& entryHiRegister, Word& entryLoRegister) : TLBInstruction(statusRegister, tlb),
    indexRegister(indexRegister), entryHiRegister(entryHiRegister), entryLoRegister(entryLoRegister) {}

void TLBWriteIndexed::run() {
    EXL_CHECK;

    assert(indexRegister < TLB_ENTRIES);

   tlb[indexRegister] = Hardware::TLBEntry( entryHiRegister, entryLoRegister );
}

TLBWriteRandom::TLBWriteRandom(TLB_INSTR_ARGS, Word& indexRegister, Word& entryHiRegister, Word& entryLoRegister) : 
    TLBWriteIndexed(statusRegister, tlb, indexRegister, entryHiRegister, entryLoRegister) {}

void TLBWriteRandom::run() {
    TLBWriteIndexed::run();
    ++indexRegister;
    indexRegister %= TLB_ENTRIES;
}

PerformCacheOp::PerformCacheOp(K_INSTR_ARGS, LRUCache<Word, std::unique_ptr<Instruction>, 32 * 1024 >& cache) : KInstruction(statusRegister), cache(cache) {}
void PerformCacheOp::run() {
    EXL_CHECK;
    cache.clear();   
}

// custom
VMTunnel::VMTunnel(K_INSTR_ARGS, Hardware::Machine& machine) : KInstruction(statusRegister), machine(machine) {}
void VMTunnel::run() {
    EXL_CHECK;
    
    auto& cpu = machine.accessCPU();

    Word reqAddr = cpu.readRegister(Binary::A0).ui;
    Word resAddr = cpu.readRegister(Binary::V0).ui;

    // this can be optimized pretty heavily once we switch to virtual memory
    Word req  = machine.readMemory().getWord( reqAddr );
    Word arg0 = machine.readMemory().getWord( reqAddr + 4 );
    Word arg1 = machine.readMemory().getWord( reqAddr + 8 );
    Word arg2 = machine.readMemory().getWord( reqAddr + 12 );

    Word res = 0;
    Word err = 0;


    // todo: do some optimizations with the string inputs
    switch (req) {
        case 1: // halt()
            machine.killed = true;

            break;

        case 2: // printString(const char*)
            for (Word i = arg0; machine.readMemory().getByte(i) != '\0'; ++i) std::cout << machine.readMemory().getByte(i);

            break;

        case 3: // printInteger(int)
            std::cout << arg0 << std::flush;

            break;
        
        case 4: // readInteger()
            std::cin >> res;

            break;

        case 5: { // fopen(const char* pathname, uint flags) -- need to consider error handling
            std::string filePath;
            for (Word i = arg0; machine.readMemory().getByte(i) != '\0'; ++i) filePath.push_back( machine.readMemory().getByte(i) ); 
            res = machine.accessFileSystem().open(filePath, arg1);

            break;
        }
        case 6: { // fread(int fd, char* buf, int nbytes)
            auto bytes = machine.accessFileSystem()[arg0]->read(arg2);
            for (Word i = 0; i < bytes.size(); ++i) machine.accessMemory().setByte(arg1 + i, bytes[i]);
            res = bytes.size();

            break;
        }
        case 7: // fwrite

        case 8: {// fseek(int fd, uint offset, int whence)
            auto& file = machine.accessFileSystem()[arg0];
            res = file->seek(arg1, arg2);
            err = file->error();
            break;
        }
        case 9: // fclose(int fd)
            machine.accessFileSystem().close(arg0);

            break;
        default:
            break;
    }

    // load back into res, err
    
    machine.accessMemory().setWord( resAddr, res     );
    machine.accessMemory().setWord( resAddr + 4, err );

}
