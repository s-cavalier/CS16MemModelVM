#include "String.h"


bool std::streq(const char* a, const char* b) {
    if (a == b) {
        // same pointer (also covers both NULL)
        return true;
    }
    if (a == nullptr || b == nullptr) {
        // exactly one is NULL
        return false;
    }
    // compare characters one by one
    while (*a != '\0' && *b != '\0') {
        if (*a != *b) {
            return false;
        }
        a++;
        b++;
    }
    // only equal if both hit '\0' at the same time
    return *a == *b;
}

std::size_t std::strlen(const char* s) {
    size_t len = 0;
    while ( s[len] != '\0' ) ++len;
    return len;
}

std::string::string() : _data(1, '\0') {}
std::string::string(const string& str) : _data(str._data) {}
std::string::string(const char* s) : _data( strlen(s) + 1 ) {
    for (size_t i = 0; i < _data.size(); ++i) _data[i] = s[i];
}
std::string::string(char c) : _data(2) {
    _data[0] = c;
    _data[1] = '\0';
}
std::string::string(string&& str) : _data(std::move(str._data)) {}

std::string& std::string::operator=(const string& str) {
    if (this == &str) return *this;
    _data = str._data;
    return *this;
}
std::string& std::string::operator=(const char* s) {
    size_t len = strlen(s) + 1;
    _data.resize(len);

    for (size_t i = 0; i < len; ++i) _data[i] = s[i];
    return *this;
}
std::string& std::string::operator=(char c) {
    _data.resize(2);
    _data[0] = c;
    _data[1] = '\0';
    return *this;
}
std::string& std::string::operator=(string&& str) {
    _data = std::move(str._data);
    return *this;
}

std::size_t std::string::size() const { return _data.size() - 1; }
std::size_t std::string::length() const { return size(); }
std::size_t std::string::capacity() const { return _data.capacity() - 1; }
void std::string::clear() { _data.clear(); _data.push_back('\0'); }
bool std::string::empty() const { return _data.size() == 1; }

char& std::string::operator[](size_t idx) {
    assert(idx < size());
    return _data[idx];
}
const char& std::string::operator[](size_t idx) const {
    assert(idx < size());
    return _data[idx];
}
char& std::string::at(size_t idx) {
    assert(idx < size());
    return _data[idx];
}
const char& std::string::at(size_t idx) const {
    assert(idx < size());
    return _data[idx];
}
char& std::string::back() {
    assert(size() > 0);
    return _data[size() - 1];
}
const char& std::string::back() const {
    assert(size() > 0);
    return _data[size() - 1];
}
char& std::string::front() {
    assert(size() > 0);
    return _data[0];
}
const char& std::string::front() const {
    assert(size() > 0);
    return _data[0];
}
std::string& std::string::operator+=(const string& str) {
    if (this == &str) return *this;
    size_t start = _data.size();
    size_t new_size = start + str.size() + 1;
    _data.resize( new_size );

    for (size_t i = start; i < new_size; ++i) _data[i] = str[i - start];
    _data[new_size] = '\0';
    return *this;
}
std::string& std::string::operator+=(const char* s) {
    size_t start = _data.size();
    size_t new_size = start + strlen(s) + 1;
    _data.resize(new_size);
    
    for (size_t i = start; i < new_size; ++i) _data[i] = s[i - start];
    _data[new_size] = '\0';
    return *this;
}
std::string& std::string::operator+=(char c) {
    _data.back() = c;
    _data.push_back('\0');
    return *this;
}
void std::string::push_back(char c) {
    _data.back() = c;
    _data.push_back('\0');
}
void std::string::pop_back() {
    assert(size() > 0);
    _data.pop_back();
    _data.back() = '\0';
}


const char* std::string::c_str() const { return _data.data(); }
const char* std::string::data() const { return _data.data(); }

std::string std::to_string(int value) {
    if (value == 0) return "0";

    std::string ret;
    bool negative = value < 0;

    while (value != 0) {
        int digit = value % 10;
        if (digit < 0) digit = -digit;
        ret.push_back(char('0' + digit));
        value /= 10;
    }

    if (negative) ret.push_back('-');

    for (size_t i = 0, j = ret.size() - 1; i < j; ++i, --j) {
        char tmp = ret[i];
        ret[i] = ret[j];
        ret[j] = tmp;
    }

    return ret;
}

std::string std::to_string(unsigned int value) {
    if (value == 0) return "0";

    std::string ret;

    while (value != 0) {
        int digit = value % 10;
        ret.push_back(char('0' + digit));
        value /= 10;
    }

    for (size_t i = 0, j = ret.size() - 1; i < j; ++i, --j) {
        char tmp = ret[i];
        ret[i] = ret[j];
        ret[j] = tmp;
    }

    return ret;
}

static inline const char* newline = "\n";

void std::println(const char* cstr) {
    std::printString(cstr);
    std::printString(newline);
}

void std::println(const std::string& str) {
    std::printString( str.c_str() );
    std::printString(newline);
}