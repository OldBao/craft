#ifndef TCP_SERVER_HPP_
#define TCP_SERVER_HPP_

#include <stdint.h>

template <typename T>
ll::TcpServer<T>::TcpServer() {
    _listener = NULL;
}


template <typename T>
ll::TcpServer<T>::~TcpServer(){
    if (_listener) delete _listener;
}

template <typename T>
int
ll::TcpServer<T>::ServeAt(const std::string &addr, uint16_t port, ll::Poller *poller) {
    int ret;
    ll::Socket *sock = new ll::Socket(AF_INET, SOCK_STREAM, 0);
    sock->SetReuseAddr(1);
    sock->SetBlocking(0);

    ret = sock->ListenOn(addr, port);
    if (ret < 0) {
        delete sock;
        return ret;
    }

    _listener = new ll::Listener<T>(sock);
    poller->AddEvent(EVENT_READ, _listener);

    poller->Loop();
}


#endif
