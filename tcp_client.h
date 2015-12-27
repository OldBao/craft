#ifndef TCPCLIENT_H_
#define TCPCLIENT_H_

#include "connector.h"

namespace ll {
    template <typename T>
    class TcpClient : public TcpConnector {
    public:
        TcpClient();
        virtual ~TcpClient();
    protected:
        virtual int OnConnected(Context *ctx);
        virtual int OnConnectFail(Context *ctx);
    };
};

#include "tcp_client.hpp"
#endif
