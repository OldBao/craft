#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <glog/logging.h>

#include "event.h"

template <typename T>
ll::Listener<T>::Listener(Socket *listenSocket) {
    _socket = listenSocket;
}

template <typename T>
ll::Listener<T>::~Listener (){
}

template <typename T>
int
ll::Listener<T>::FD(){
    assert(_socket);

    return _socket->fd;
}
    

template <typename T>
int
ll::Listener<T>::OnRead(Context *ctx) {
    DLOG(INFO) << "OnAccept";

    int newfd;
    ll::Socket *new_sock;
    
    new_sock = _socket->Accept();
    if (!new_sock) {
        return -1;
    }

    DLOG (INFO) << "got new connection";

    new_sock->SetBlocking(0);
    ll::Connection *conn = new T(new_sock);
    ctx->poller->AddEvent(EVENT_READ, conn, 3000);
    
    return 0;
}



