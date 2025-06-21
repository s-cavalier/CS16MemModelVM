#ifndef M__STRING_H__
#define M__STRING_H__
#include "Vector.h"

namespace ministl {

    bool streq(const char* l, const char* r);

    // rework later
    
    class string {
    private:
        ministl::vector<char> data;  // always null-terminated

    public:
        // -- Constructors / Assignment --

        string() {
            // start as empty string with just a '\0'
            data.push_back('\0');
        }

        string(const char* cstr) {
            // build from NUL-terminated char*
            for (const char* p = cstr; *p != '\0'; ++p) {
                data.push_back(*p);
            }
            data.push_back('\0');
        }

        string(const string& other)
            : data(other.data)
        {}

        string& operator=(const string& other) {
            if (this != &other) {
                data = other.data;
            }
            return *this;
        }

        string(string&& other) noexcept
            : data(ministl::move(other.data))
        {}

        string& operator=(string&& other) noexcept {
            if (this != &other) {
                data = ministl::move(other.data);
            }
            return *this;
        }

        // -- Element access / c-string --

        const char* c_str() const {
            return data.data();
        }

        char& operator[](size_t idx) {
            return data[idx];
        }

        const char& operator[](size_t idx) const {
            return data[idx];
        }

        // -- Capacity --

        size_t size() const {
            return (data.size() > 0 ? data.size() - 1 : 0);
        }

        bool empty() const {
            return size() == 0;
        }

        void clear() {
            data.clear();
            data.push_back('\0');
        }

        // -- Modifiers --

        void push_back(char ch) {
            // remove old null, append ch, then restore null
            data.pop_back();
            data.push_back(ch);
            data.push_back('\0');
        }

        string& operator+=(const string& other) {
            // remove our null, copy others' chars, then add null
            data.pop_back();
            for (size_t i = 0; i < other.size(); ++i) {
                data.push_back(other[i]);
            }
            data.push_back('\0');
            return *this;
        }

        string& operator+=(char x) {
            // remove our null, copy others' chars, then add null
            push_back(x);
            return *this;
        }

        // -- Comparison --

        bool operator==(const string& other) const {
            if (size() != other.size()) return false;
            for (size_t i = 0; i < size(); ++i) {
                if (data[i] != other.data[i]) return false;
            }
            return true;
        }

        bool operator!=(const string& other) const {
            return !(*this == other);
        }

        // -- Concatenation --

        friend string operator+(string lhs, const string& rhs) {
            lhs += rhs;
            return lhs;
        }
    };

    

}

#endif