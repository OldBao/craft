#ifdef LL_DEBUG
#include <glog/logging.h>
#include <sstream>
#include <iostream>
#endif

#include <errno.h>

#include "buffer.h"

#define TRUNK_SIZE 4096
using namespace ll;

Buffer::Buffer(){
    _start = _allocTrunk(TRUNK_SIZE);
    _end = _start;
}

Buffer::~Buffer(){
    Trunk *p = _start, *q;
    
    while(p) {
        q = p->next;
        free(p);
        p = q;
    };
}

ssize_t
Buffer::WriteToFD(int fd, size_t total) {
    size_t max, left = total, ret, expect, writed = 0;
    Trunk *p = GetReadableTrunk();;
    left = total;

    do {
        expect = p->writePos - p->readPos;
        if (expect == 0) break;

        if (total != 0) {
            expect = expect > left ? expect : left;
        }

        ret = write(fd, (char *) p->base + p->readPos, expect);
        if (ret < 0) {
            if (errno == EAGAIN) {
                break;
            } else {
                return ret;
            }
        } else {
            if (total != 0) {
                left -= ret;
                assert(left > 0);
                if (left == 0)
                    break;
            }
            writed += ret;
            p->readPos += ret;
        }

        p = p->next;
    } while (p);

#ifdef LL_DEBUG
    __dumpBuffer();
#endif

    return writed;
}


ssize_t
Buffer::ReadFromFD(int fd, size_t total) {
    size_t left = total, expect, writed = 0;
    Trunk *p = GetWritableTrunk();
    int ret;
    
    do {
        expect = p->capacity - p->writePos;
        if (total != 0) {
            expect = expect < left ? expect : left;
        }
        
        ret = read(fd, (char *) p->base + p->writePos, expect);
        if (ret < 0) {
            if (errno == EAGAIN) {
                break;
            } else {
                return ret;
            }
        } else if (ret == 0) {
            break;
        }else {
            if (total != 0) {
                left -= ret;
                if (left == 0) break;
            }
            
            writed += ret;
            p->writePos += ret;
        }

        p = GetWritableTrunk();
    } while (1);

#ifdef LL_DEBUG
    __dumpBuffer();
#endif

    return writed;
}

ssize_t 
Buffer::WriteToBuffer(Buffer *other, size_t total) {
    size_t left = total, leftp, leftq, writed;
    Trunk *p = GetReadableTrunk(), *q = other->GetWritableTrunk();
    
    do {
        if (p->readPos == p->writePos) {
            break;
        }

        leftp = p->writePos - p->readPos;
        leftq = q->capacity - q->writePos;
        leftp = leftp < leftq ? leftp : leftq;

        if (total != 0) {
            leftp = leftp < left ? leftp : left;
        }

        memcpy((char *)q->base + q->writePos, 
               (char *)p->base + p->readPos, 
               leftp);
        
        q->writePos += leftp;
        p->readPos += leftp;
        
        if (total != 0) {
            left -= leftp;
            assert(left > 0);
            if (left == 0) {
                break;
            }
        }

        writed += leftp;
        p = p->next;
        q = other->GetWritableTrunk();
    } while(p);

#ifdef LL_DEBUG
    __dumpBuffer();
    other->__dumpBuffer();
#endif
    return writed;
}


Trunk*
Buffer::_allocTrunk(ssize_t len){
    assert (len > sizeof(Trunk));

    Trunk *t = (Trunk *) malloc(len);
    t->readPos = t->writePos = 0;
    t->capacity = len - sizeof(Trunk)+1;
    t->next = NULL;
    return t;
}


Trunk *
Buffer::GetWritableTrunk() {
    if (_end->writePos == _end->capacity) {
        _end->next = _allocTrunk(TRUNK_SIZE);
        _end = _end->next;
    }

    return _end;
}


Trunk *
Buffer::GetReadableTrunk(){
    Trunk *p = _start;

    do {
        if (p == _end || p->readPos != p->writePos) break;
        p = p->next;
    } while (1);

    _start = p;
    return p;
}


ssize_t 
Buffer::WriteToString(char *buf, ssize_t len) {
    Trunk *p = GetReadableTrunk();
    size_t left = len, expect;

    while(left) {
        expect = p->writePos - p->readPos;
        if (expect == 0) break;
        expect = expect < left ? expect : left;
        memcpy(buf+len-left, (char *)p->base+p->readPos, expect);
        left -= expect;
        p->readPos += expect;
        p = GetReadableTrunk();
    }

    return len - left;
}


ssize_t
Buffer::WriteToString(std::string *msg) {
    size_t ret;
    char buf[4096];
    
    while (1) {
        ret = WriteToString((char *)buf, (ssize_t)4096);
        msg->append(buf, ret);

        if (ret < 4096)  {
            break;
        }
    }
    return msg->length();
}

ssize_t
Buffer::ReadFromString(const char *buf, ssize_t len) {
    Trunk *p = GetWritableTrunk();
    size_t left = len, expect;

    while (left) {
        expect = p->capacity - p->writePos;
        expect = expect < left ? expect : left;

        memcpy((char *)p->base + p->writePos, buf, expect);
        
        p->writePos += expect;
        p = GetWritableTrunk();
        left -= expect;
    }

    return len - left;
}

ssize_t
Buffer::ReadFromString(const std::string &msg) {
    return ReadFromString(msg.c_str(), msg.length());
}

#ifdef LL_DEBUG
void Buffer::__dumpBuffer() {
    Trunk *p = GetReadableTrunk();
    int i = 1;
    do {

        LOG(INFO) << "Trunk "<< i++ << ": " << "ReadPos: " << p->readPos << 
            " WritePos: " << p->writePos << "\t";
        p = p->next;
    } while (p);
}

size_t
ll::Buffer::GetReadableLen(){
    Trunk *p = GetReadableTrunk();
    size_t len = 0;

    do {
        if (p->readPos == p->writePos) break;
        len += p->writePos - p->readPos;
        p = p->next;
    } while (p);

    return len;
}
#endif
