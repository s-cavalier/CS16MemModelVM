#include "Loader.h"
#include "BinaryUtils.h"
#include <fstream>

 std::vector<unsigned char>& FileLoader::Parser::readData()  {
    return data;
}

 std::vector<unsigned int>& FileLoader::Parser::readText()  {
    return text;
}

const bool& FileLoader::Parser::bad() const {
    return _bad;
}

FileLoader::ExecutableParser::ExecutableParser(const std::string& path) : Parser() {
    std::ifstream file(path);
    if (file.fail()) {
        _bad = true;
        return;
    }

    Byte buffer[4];
    while (file.read((char*)buffer, 4)) text.push_back(Binary::loadBigEndian(buffer));
}
