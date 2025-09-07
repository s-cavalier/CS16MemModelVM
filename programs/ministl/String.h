#ifndef M__STRING_H__
#define M__STRING_H__
#include "Vector.h"

namespace std {

    bool streq(const char* l, const char* r);
    size_t strlen(const char* s);

    class string {
        std::vector<char> _data;
    
    public:
        static constexpr size_t npos = -1;

        string();
        string(const string& str);
        string(const char* s);
        string(char c);
        string(string&& str);

        string& operator=(const string& str);
        string& operator=(const char* s);
        string& operator=(char c);
        string& operator=(string&& str);

        ~string() = default;

        size_t size() const;
        size_t length() const;
        size_t capacity() const;
        void clear();
        bool empty() const;

        char& operator[](size_t idx);
        const char& operator[](size_t idx) const;
        char& at(size_t idx);
        const char& at(size_t idx) const;
        char& front();
        const char& front() const;
        char& back();
        const char& back() const;

        string& operator+=(const string& str);
        string& operator+=(const char* s);
        string& operator+=(char c);
        void push_back(char c);
        void pop_back();

        const char* c_str() const;
        const char* data() const;
        
    };

    string to_string(int value);
    string to_string(unsigned int value);

    void println(const char* cstr);
    void println(const string& str);

    
    


}

#endif