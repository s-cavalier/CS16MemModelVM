#include "String.h"


bool ministl::streq(const char* a, const char* b) {
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

ministl::size_t ministl::strlen(const char* s) {
    size_t len = 0;
    while ( s[len] != '\0' ) ++len;
    return len;
}

ministl::string::string() : _data(1, '\0') {}
ministl::string::string(const string& str) : _data(str._data) {}
ministl::string::string(const char* s) : _data( strlen(s) + 1 ) {
    for (size_t i = 0; i < _data.size(); ++i) _data[i] = s[i];
}
ministl::string::string(char c) : _data(2) {
    _data[0] = c;
    _data[1] = '\0';
}
ministl::string::string(string&& str) : _data(ministl::move(str._data)) {}

ministl::string& ministl::string::operator=(const string& str) {
    if (this == &str) return *this;
    _data = str._data;
    return *this;
}
ministl::string& ministl::string::operator=(const char* s) {
    size_t len = strlen(s) + 1;
    _data.resize(len);

    for (size_t i = 0; i < len; ++i) _data[i] = s[i];
    return *this;
}
ministl::string& ministl::string::operator=(char c) {
    _data.resize(2);
    _data[0] = c;
    _data[1] = '\0';
    return *this;
}
ministl::string& ministl::string::operator=(string&& str) {
    _data = ministl::move(str._data);
    return *this;
}

ministl::size_t ministl::string::size() const { return _data.size() - 1; }
ministl::size_t ministl::string::length() const { return size(); }
ministl::size_t ministl::string::capacity() const { return _data.capacity() - 1; }
void ministl::string::clear() { _data.clear(); _data.push_back('\0'); }
bool ministl::string::empty() const { return _data.size() == 1; }

char& ministl::string::operator[](size_t idx) {
    assert(idx < size());
    return _data[idx];
}
const char& ministl::string::operator[](size_t idx) const {
    assert(idx < size());
    return _data[idx];
}
char& ministl::string::at(size_t idx) {
    assert(idx < size());
    return _data[idx];
}
const char& ministl::string::at(size_t idx) const {
    assert(idx < size());
    return _data[idx];
}
char& ministl::string::back() {
    assert(size() > 0);
    return _data[size() - 1];
}
const char& ministl::string::back() const {
    assert(size() > 0);
    return _data[size() - 1];
}
char& ministl::string::front() {
    assert(size() > 0);
    return _data[0];
}
const char& ministl::string::front() const {
    assert(size() > 0);
    return _data[0];
}
ministl::string& ministl::string::operator+=(const string& str) {
    if (this == &str) return *this;
    size_t start = _data.size();
    size_t new_size = start + str.size() + 1;
    _data.resize( new_size );

    for (size_t i = start; i < new_size; ++i) _data[i] = str[i - start];
    _data[new_size] = '\0';
    return *this;
}
ministl::string& ministl::string::operator+=(const char* s) {
    size_t start = _data.size();
    size_t new_size = start + strlen(s) + 1;
    _data.resize(new_size);
    
    for (size_t i = start; i < new_size; ++i) _data[i] = s[i - start];
    _data[new_size] = '\0';
    return *this;
}
ministl::string& ministl::string::operator+=(char c) {
    _data.back() = c;
    _data.push_back('\0');
    return *this;
}
void ministl::string::push_back(char c) {
    _data.back() = c;
    _data.push_back('\0');
}
void ministl::string::pop_back() {
    assert(size() > 0);
    _data.pop_back();
    _data.back() = '\0';
}


const char* ministl::string::c_str() const { return _data.data(); }
const char* ministl::string::data() const { return _data.data(); }