#include <fcntl.h>
#include <unistd.h>

#include "tcp_client.h"
#include "event.h"

using namespace ll;
class EchoConnection;

class StdioEvent : public Event{
public:
    StdioEvent(EchoConnection *connection);
    ~StdioEvent();
    virtual int FD();
    virtual int OnRead(Context *ctx);

private:
    TcpConnection *_conn;
};

class EchoConnection : public TcpConnection {
public:
    EchoConnection(Socket *sock) :
        TcpConnection(sock){}

    ~EchoConnection() {

    }
    virtual int OnCreated(Context *ctx) {
        DLOG(INFO) << "OnCreated";
        StdioEvent *ev = new StdioEvent(this);
        ctx->poller->AddEvent(EVENT_READ, ev);
    }

    virtual int OnNewMessage(Context *ctx) {
        DLOG(INFO) << "OnNewMessage";
        std::string hehe;
        Recv(ctx, &hehe);
        DLOG(INFO) << "Readed" << hehe;
    }

    virtual int OnClose(Context *ctx){
        DLOG(INFO) << "OnConnectionClose";
        exit(0);
    }
};


StdioEvent::StdioEvent(EchoConnection *connection) {
    _conn = connection;
    int flags;
    
    flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(STDIN_FILENO, F_SETFL, flags);
}

int StdioEvent::FD() {
    return STDIN_FILENO; 
}


int StdioEvent::OnRead(Context *ctx){
    Buffer buffer;
    std::string test;

    buffer.ReadFromFD(STDIN_FILENO);
    buffer.WriteToString(&test);
    _conn->Send(ctx, test);
}

int
main(int argc, char **argv) {
    google::InitGoogleLogging(*argv);

    Epoller poller;
    TcpClient<EchoConnection> client;
    client.ConnectTo(&poller, "0.0.0.0", 8080);

    poller.Loop();
    return 0;
}
