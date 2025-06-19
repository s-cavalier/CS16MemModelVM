#include "FileSystem.h"
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

// to better error handling here

Hardware::OpenFile::OpenFile(const std::string& filePath, Word flags) {
    fd = open(filePath.c_str(), flags);
    offset = 0;
    
    err = 0;
    if (fd == Word(-1)) err = errno;
}

std::vector<Byte> Hardware::OpenFile::read(Word count) {
    std::vector<Byte> buffer(count);
    DoubleWord bytesRead = ::read(fd, buffer.data(), count);

    if (bytesRead == Word(-1)) {
        err = errno;
        buffer.clear();
    } 
    else buffer.resize(bytesRead);

    return buffer;
}

Hardware::OpenFile::~OpenFile() {
    assert((close(fd) == 0) && "Recieved an error on fileclose"); // should be a valid file
}


Hardware::FileSystem::FileSystem() {}

Word Hardware::FileSystem::open(const std::string& filePath, Word flags) {
    if (freeVFDs.empty()) { // if no free vfds are available
        Word vfd = files.size();
        files.emplace_back(std::in_place, filePath, flags);
        return vfd;
    }
    
    // if free vfds are available

    Word vfd = freeVFDs.back();
    freeVFDs.pop_back();

    auto& file = files[vfd];
    assert(!file && "Reusing occupied VFD");               // should only be able to access "empty" file slots

    file = std::make_optional<OpenFile>(filePath, flags);

    if (file->error()) return file->error();

    return vfd;
}

// TODO: better error handling
int Hardware::FileSystem::close(Word vfd) {
    if (vfd >= files.size()) return -1;
    auto& file = files[vfd];
    if (!file) return -2;
    file.reset();
    freeVFDs.push_back(vfd);
    return 0;
}

std::optional<Hardware::OpenFile>& Hardware::FileSystem::operator[](Word idx) {
    return files.at(idx);
}

const std::optional<Hardware::OpenFile>& Hardware::FileSystem::operator[](Word idx) const {
    return files.at(idx);
}