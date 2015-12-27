#ifndef BUFFER_H_
#define BUFFER_H_

#include <string>
#include <sys/types.h>
namespace ll {
    struct Trunk {
        ssize_t readPos, writePos;
        ssize_t capacity;
        struct Trunk *next;
        char base[1];
    };

    
    class Buffer {
    public:
        Buffer();
        virtual ~Buffer();

        ssize_t WriteToFD(int fd, size_t len = 0);
        ssize_t ReadFromFD(int fd, size_t len = 0);

        ssize_t ReadFromString(const char *msg, ssize_t len);
        ssize_t ReadFromString(const std::string &msg);
        ssize_t WriteToString(char *msg, ssize_t len);
        ssize_t WriteToString(std::string *buf);

        ssize_t ReadFromBuffer(Buffer &other, size_t len = 0);
        ssize_t WriteToBuffer(Buffer *other, size_t len = 0);
        Trunk *GetWritableTrunk();
        Trunk *GetReadableTrunk();

        size_t GetReadableLen();
    private:
        void __dumpBuffer();
        Trunk *_allocTrunk(ssize_t len);
        Trunk *_start, *_end;
    };

};
#endif
