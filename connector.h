#ifndef CONNECTOR_H_
#define CONNECTOR_H_
#include <glog/logging.h>
#include "event.h"
namespace ll {
    enum ConnectState {
        DISCONN,
        CONNECTING,
        CONNECTED,
    };

    class TcpConnector : public Event {
    public:
        TcpConnector(int timeout=3000, int retries=3);
        ~TcpConnector();

        virtual int ConnectTo(Poller *poller, const std::string &addr, uint16_t port);
        virtual int Disconnect(Poller *poller);

        virtual int FD() {return _socket->fd;}

        virtual int OnWrite(Context *ctx);
        virtual int OnHup(Context *ctx);
        virtual int OnError(Context *ctx);
        virtual int OnTimeout(Context *ctx);

        virtual int OnConnected(Context *ctx) = 0;
        virtual int OnConnectFail(Context *ctx) = 0;
        
    private:
        int _handleFails(Context *ctx);
        Socket *_socket;
        Connection *_connection;
        int _timeout, _retries, _try;
        int _state;

        std::string _addr;
        uint16_t _port;
    };
};
#endif
