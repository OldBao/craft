#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include "socket.h"
#include <glog/logging.h>
#include <netinet/tcp.h>

using namespace ll;

static int
_getAddr(const std::string &addr, struct sockaddr *retaddr, size_t addr_size, int af) {
    struct addrinfo hints, *res;
    int ret;
    int d[4];
    char ch;

    if (sscanf(addr.c_str(), "%d.%d.%d.%d%c", &d[0], &d[1], &d[2], &d[3], &ch) == 4
        && 0 <= d[0] && d[0] <= 255 && 0 <= d[1] && d[1] <= 255 
        && 0 <= d[2] && d[2] <= 255 && 0 <= d[3] && d[3] <= 255) {
        struct sockaddr_in *sin =(struct sockaddr_in *) retaddr;
        sin->sin_addr.s_addr = htonl (
                                      ((long) d[0] << 24) | ((long) d[1] << 16) |
                                      ((long) d[2] << 8) | ((long) d[3] << 0)
                                      );
        sin->sin_family = AF_INET;
        return 4;
    }

    // named addr
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = af;
    ret = getaddrinfo(addr.c_str(), NULL, &hints, &res);
    if (ret < 0) {
        PLOG(WARNING) << "getaddrinfo " << addr << " error";
        return -1;
    }

    if (res->ai_addrlen < addr_size) {
        //check ret buffer
        addr_size = res->ai_addrlen;
    }
    memcpy((char *) retaddr, res->ai_addr, addr_size);
    freeaddrinfo(res);

    if (retaddr->sa_family != AF_INET) {
        PLOG(WARNING) << "unknown addr family " << retaddr->sa_family;
        return -1;
    }
    return 4;
}

static int
_getIPAddr(std::string *addr, struct sockaddr *sock, socklen_t len) {
    int ret;
    char buf[4096];

    ret = getnameinfo(sock, len, buf, sizeof(buf), NULL, 0, NI_NUMERICHOST);
    if (ret) {
        PLOG(WARNING) << "getnameinfo";
        return -1;
    }
    
    addr->assign(buf);
    return 0;
}



Socket::Socket(int newfd, struct sockaddr *addrbuf, socklen_t addrlen) {
    fd = newfd;
    if (addrlen > sizeof (struct sockaddr)) {
        addrlen = sizeof (struct sockaddr);
    }

    memcpy((char *)&_sockaddr, addrbuf, addrlen);

}


Socket::Socket(int family, int type, int proto):
    _family(family), _type(type), _proto(proto){
    fd = socket(_family, _type, _proto);
    if (fd < 0) {
        PLOG(INFO) << "create socket error";
        throw new SocketException("create socket error");
    }
}


Socket::~Socket(){
    if (fd > 0) close(fd);
}


Socket*
Socket::Accept(){
    int newfd;
    struct sockaddr addrbuf;
    socklen_t addrlen = sizeof (struct sockaddr);

    newfd = accept(fd, &addrbuf, &addrlen);
    if (newfd < 0) {
        PLOG(WARNING) << "accept error";
        return NULL;
    }
    
    Socket *socket = new Socket(newfd, &addrbuf, addrlen);
    return socket;
}


int
Socket::SetNoDelay(int nodelay) {
    nodelay = nodelay != 0;
    return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, 
                      &nodelay, sizeof(int));
}


int
Socket::SetKeepAlive(int kl) {
    kl = kl != 0;
    return setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, 
                      &kl, sizeof(int));
}


int
Socket::SetReuseAddr(int enable) {
    enable = enable != 0;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)&enable, sizeof(int));
}

int
Socket::SetBlocking(int block){
    assert (fd > 0);

    int flags, ret;
    
    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        PLOG(WARNING) << "get flag error";
        return -1;
    }

    if (block) {
        flags &=  (~O_NONBLOCK);
    } else {
        flags |= O_NONBLOCK;
    }
    return fcntl(fd, F_SETFL, flags);
}


int
Socket::ListenOn(const std::string& addr, uint16_t port, int backlog) {
    struct sockaddr_in in_addr;
    int ret;
    
    ret = _getAddr(addr, (struct sockaddr *)&in_addr, sizeof(in_addr), AF_INET);
    if (ret < 0) {
        PLOG(WARNING) << "get addr error";
        return -1;
    }
    in_addr.sin_port = htons(port);


    ret = bind(fd, (struct sockaddr *) &in_addr, sizeof(in_addr));
    if (ret < 0) {
        PLOG(WARNING) << "bind error";
        return -1;
    }

    ret = listen(fd, backlog);
    if (ret < 0) {
        PLOG(WARNING) << "listen error";
        return -1;
    }

    return 0;
}


int
Socket::Connect(const std::string &remoteAddr, uint16_t port) {
    struct sockaddr_in addr;
    int ret;
    
    ret = _getAddr(remoteAddr, 
                   (struct sockaddr *)&addr, sizeof(struct sockaddr), AF_INET);
    if (ret < 0) {
        return ret;
    }

    addr.sin_port = htons(port);
    
    return Connect((struct sockaddr *)&addr, (socklen_t)sizeof(addr));
}


int
Socket::Connect(struct sockaddr *addr, socklen_t addrlen) {
    int ret;

    return connect(fd, addr, addrlen);
}
