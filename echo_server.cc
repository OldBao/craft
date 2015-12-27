#include <glog/logging.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "event.h"
#include "socket.h"
#include "tcp_server.h"
#include "tcp_connection.h"

using namespace ll;

class EchoConnection : public TcpConnection {
public:
    EchoConnection(Socket *sock):
        TcpConnection(sock){
    }


    virtual 
    int OnRead(Context *ctx) {
        LOG (INFO) << "client OnRead";
        int ret;

        ret = _readBuffer.ReadFromFD(_socket->fd);
        if (ret < 0 && errno == EAGAIN) {
            ctx->poller->AddEvent(EVENT_READ, this);
            return ret;
        }

        DLOG(INFO) << "Got : " << ret << "bytes";
        if (ret == 0) {
            return 0;
        }
        
        _readBuffer.WriteToBuffer(&_writeBuffer);
        
        ctx->poller->AddEvent(EVENT_WRITE, this);
        return 0;
    }


    virtual
    int OnWrite(Context *ctx) {
        LOG(INFO) << "client OnWrite";
        int ret;

        ret = _writeBuffer.WriteToFD(_socket->fd);
        
        if (ret < 0 && errno == EAGAIN) {
            ctx->poller->AddEvent(EVENT_WRITE, this);
        }

        DLOG(INFO) << "Write To Client : " << ret << "bytes";
        ctx->poller->DelEvent(EVENT_WRITE, this);

        return 0;
    }
  
    virtual int OnCreated(Context *ctx) {

    }
    virtual int OnNewMessage(Context *ctx) {

    }

    virtual int OnClose(Context *ctx) {

    }

    virtual
    int OnHup (Context *ctx){
        LOG(INFO) << "client Hup";
    }


    virtual
    int OnUrgent(Context *ctx){
        LOG(INFO) << "client Urgent";
    }


    virtual
    int OnError(Context *ctx) {
        LOG(INFO) << "client Error";
    }

    virtual
    int OnTimeout(Context *ctx) {
        LOG(INFO) << "event timeout";

        ctx->poller->DelEvent(EVENT_ALL, this);
        ctx->poller->AddEvent(EVENT_READ, this, 3000);
    }
};


int
main(int argc, char **argv) {
    google::InitGoogleLogging(*argv);

    Poller *poller = new Epoller();
    TcpServer<EchoConnection> *server = new TcpServer<EchoConnection>();
    server->ServeAt("0.0.0.0", 12345, poller);
    poller->Loop();
    return 0;
}
