#include "Loader.h"
#include <fstream>
#include <elfio/elfio.hpp>

inline unsigned int loadBigEndian(const unsigned char bytes[4]) {
    return (((unsigned int)(bytes[0])) << 24) |
        (((unsigned int)(bytes[1])) << 16) |
        (((unsigned int)(bytes[2])) << 8)  |
        (((unsigned int)(bytes[3]))); 
}

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
    while (file.read((char*)buffer, 4)) text.push_back(loadBigEndian(buffer));
}

FileLoader::ELFLoader::ELFLoader(const std::string& path) : Parser() {
    ELFIO::elfio reader;

    if (!reader.load(path)) {
        _bad = true;
        return;
    }

    // 2) Load .text section into vector<unsigned int>
    const ELFIO::section* text_sec = reader.sections[".text"];
    if (text_sec) {
        const char*       bytes = text_sec->get_data();
        std::size_t       sz    = text_sec->get_size();
        std::size_t       count = sz / sizeof(unsigned int);
        text.reserve(count);

        for (std::size_t i = 0; i < count; ++i) {
            const unsigned char* p = reinterpret_cast<const unsigned char*>(bytes + i * sizeof(unsigned int));
            unsigned int         w;

            if (reader.get_encoding() == ELFIO::ELFDATA2LSB) {
                // little-endian
                w =  (unsigned int)p[0]
                   | (unsigned int)p[1] <<  8
                   | (unsigned int)p[2] << 16
                   | (unsigned int)p[3] << 24;
            } else {
                // big-endian
                w =  (unsigned int)p[0] << 24
                   | (unsigned int)p[1] << 16
                   | (unsigned int)p[2] <<  8
                   | (unsigned int)p[3];
            }

            text.push_back(w);
        }
    }

    // 3) Load .data section into vector<unsigned char>
    const ELFIO::section* data_sec = reader.sections[".data"];
    if (data_sec) {
        const char*       bytes = data_sec->get_data();
        std::size_t       sz    = data_sec->get_size();
        
        data.assign(
            reinterpret_cast<const unsigned char*>(bytes),
            reinterpret_cast<const unsigned char*>(bytes) + sz
        );
    }
}
