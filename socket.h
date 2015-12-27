#ifndef NETWORKING_H_
#define NETWORKING_H_

#include <string>
#include <exception>
#include <sys/socket.h>

namespace ll {

    class SocketException : public std::exception {
    public:
        SocketException(const std::string &msg) {_msg = msg;}
        virtual ~SocketException() throw() {}
        
        virtual const char *what() const throw(){
            return _msg.c_str();
        }
    private:
        std::string _msg;
    };

    class Socket {
    public:
        int fd;

        Socket(int family, int type, int proto);
        Socket(int newfd, struct sockaddr *addrbuf, socklen_t addrlen);
        virtual ~Socket();

        int ListenOn(const std::string &addr, uint16_t port, int backlog=1024);
        int Connect(const std::string &addr, uint16_t port);
        int Connect(struct sockaddr *addr, socklen_t addrlen);
        int Close();
        Socket *Accept();
        int SetBlocking(int blocked);
        int SetReuseAddr(int enable);
        int SetNoDelay(int nodelay);
        int SetKeepAlive(int keepalive);

        const std::string GetRemoteAddr();
        uint16_t GetRemotePort();

    private:
        int _family, _proto, _type;
        struct sockaddr _sockaddr;
        
        int _timeout;
        int _nonblocking;
    };
};

#endif
