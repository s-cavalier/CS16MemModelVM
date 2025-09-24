#include "asmio.h"
#include "String.h"

namespace std {
    class COstream {
        static constexpr size_t MAX_BUF = 4096;
        char* streambuf;
        size_t bufsize;

        COstream();

        void appendbuf(char c);

    public:
        ~COstream();

        COstream(const COstream& other) = delete;
        COstream(COstream&& other) = delete;
        COstream& operator=(const COstream& other) = delete;
        COstream& operator=(COstream&& other) = delete;

        void flush();

        COstream& operator<<(const string& other);
        COstream& operator<<(const char* other);
        COstream& operator<<(size_t other);
        COstream& operator<<(int other);
        COstream& operator<<(char other);

        using StreamManipulator = void (*) (COstream&);
        COstream& operator<<(StreamManipulator manip);

        static COstream instance;
    };

    inline COstream& cout = COstream::instance;

    void flush(COstream& stream);

    void endl(COstream& stream);

    class CIstream {
        


    };

}

