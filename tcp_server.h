#ifndef TCPSERVER_H_
#define TCPSERVER_H_
#include "event.h"
#include "buffer.h"

namespace ll {
    template <typename T>
    class TcpServer {
    public:
        TcpServer();
        virtual ~TcpServer();

        int ServeAt(const std::string& addr, uint16_t port, Poller *poller);

    protected:
        Listener<T> *_listener;
    };
};

#include "tcp_server.hpp"

#endif
