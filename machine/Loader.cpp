#include "Loader.h"
#include "BinaryUtils.h"
#include <fstream>
#include <elf.h>
#include <iostream>

FileLoader::Parser::Parser() : _bad(false) {}

 std::vector<unsigned char>& FileLoader::Parser::readData()  {
    return data;
}

 std::vector<unsigned int>& FileLoader::Parser::readText()  {
    return text;
}

const bool& FileLoader::Parser::bad() const {
    return _bad;
}

FileLoader::SpimLoader::SpimLoader(const std::string& path) : Parser() {
    std::ifstream file(path);
    if (file.fail()) {
        _bad = true;
        return;
    }

    Byte buffer[4];
    while (file.read((char*)buffer, 4)) text.push_back(Binary::loadBigEndian(buffer));
}

FileLoader::ELFLoader::ELFLoader(const std::string& path) : Parser() {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        _bad = true;
        return;
    }

    // Read ELF header
    Elf32_Ehdr ehdr;
    file.read(reinterpret_cast<char*>(&ehdr), sizeof(ehdr));
    if (!file) {
        _bad = true;
        return;
    }

    // Verify ELF magic number
    if (ehdr.e_ident[EI_MAG0] != ELFMAG0 || 
        ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
        ehdr.e_ident[EI_MAG2] != ELFMAG2 || 
        ehdr.e_ident[EI_MAG3] != ELFMAG3) {
        _bad = true;
        return;
    }

    std::cout << "Valid ELF file.\n";
    std::cout << "Entry point: 0x" << std::hex << ehdr.e_entry << '\n';
    std::cout << "Program header offset: " << std::dec << ehdr.e_phoff << '\n';
    std::cout << "Section header offset: " << ehdr.e_shoff << '\n';
    std::cout << "Number of section headers: " << ehdr.e_shnum << '\n';
    std::cout << "Section header string table index: " << ehdr.e_shstrndx << '\n';

}