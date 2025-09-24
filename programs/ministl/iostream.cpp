#include "iostream.h"

std::COstream std::COstream::instance;

std::COstream::COstream() {
    streambuf = new char[MAX_BUF + 1];
    bufsize = 0;
}

void std::COstream::appendbuf(char c) {
    if (bufsize == MAX_BUF) flush();
    streambuf[bufsize++] = c;
}

std::COstream::~COstream() {
    delete[] streambuf;
}

void std::COstream::flush() {
    if (streambuf[bufsize] != '\0') streambuf[bufsize] = '\0';
    std::printString(streambuf);
    bufsize = 0;
}

std::COstream& std::COstream::operator<<(const string& other) {
    for (size_t i = 0; i < other.size(); ++i) appendbuf( other[i] );
    return *this;
}

std::COstream& std::COstream::operator<<(const char* other) {
    for (size_t i = 0; other[i] != '\0'; ++i) appendbuf( other[i] );
    return *this;
}

std::COstream& std::COstream::operator<<(size_t other) {
    std::string x = std::to_string(other);
    for (size_t i = 0; i < x.size(); ++i) appendbuf( x[i] );
    return *this;
}

std::COstream& std::COstream::operator<<(int other) {
    std::string x = std::to_string(other);
    for (size_t i = 0; i < x.size(); ++i) appendbuf( x[i] );
    return *this;
}

std::COstream& std::COstream::operator<<(char c) {
    appendbuf(c);
    return *this;
}

std::COstream& std::COstream::operator<<(StreamManipulator manip) {
    manip(*this);
    return *this;
}

void std::flush(COstream& stream) {
    stream.flush();
}

void std::endl(COstream& stream) {
    stream << '\n';
    stream.flush();
}