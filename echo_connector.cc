#include <iostream>

#include "event.h"
#include "connector.h"
#include "tcp_connection.h"
#include "buffer.h"

using namespace ll;
using namespace std;

class EchoConnection: public TcpConnection {
    
public:
    EchoConnection(Socket *sock):
        TcpConnection(sock) {

    }

    int Send(Context *ctx, const std::string &msg) {
        _writeBuffer.ReadFromString(msg);
        ctx->poller->AddEvent(EVENT_WRITE, this);
    }

    int OnWrite(Context *ctx) {
        LOG(INFO) << "client OnWrite";
        int ret;

        ret = _writeBuffer.WriteToFD(_socket->fd);
        
        if (ret < 0 && errno == EAGAIN) {
            ctx->poller->AddEvent(EVENT_WRITE, this);
        }

        DLOG(INFO) << "Write To Server : " << ret << "bytes";
        ctx->poller->AddEvent(EVENT_READ, this, 3000, 1);

        return 0;
    }

    virtual int OnRead(Context *ctx) {
        LOG (INFO) << "client OnRead";
        int ret;
        std::string buf;

        ret = _readBuffer.ReadFromFD(_socket->fd);
        if (ret == 0) {
            return 0;
        }

        DLOG(INFO) << "Got : " << ret << "bytes";
        _readBuffer.WriteToString(&buf);

        DLOG(INFO) << "Readed : " << buf;

        ctx->poller->CancelTimeout(this);
        ctx->poller->DelEvent(EVENT_READ, this);
    }
};


class EchoConnector: public TcpConnector {
public:
    EchoConnector(int timeout=3000, int retry=3):
        TcpConnector(timeout, retry) {}
    virtual ~EchoConnector(){}

    int OnConnected(Context *ctx){
        LOG(INFO) << "Connected";

        EchoConnection *newConn = new EchoConnection(ctx->socket);
        newConn->Send(ctx, "GET / HTTP/1.1\r\n\r\n");
    }

    int OnConnectFail(Context *ctx) {
        LOG(INFO) << "Connect Fail";
    }
};


int
main(int argc, char **argv) {
    google::InitGoogleLogging(*argv);
    
    Poller *poller = new Epoller();

    TcpConnector *connector = new EchoConnector();
    //connector->ConnectTo(poller, "localhost", 8080);
    connector->ConnectTo(poller, "www.baidu.com", 80);

    poller->Loop();
    return 0;
}


