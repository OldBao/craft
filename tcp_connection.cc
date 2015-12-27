#include "tcp_connection.h"

ll::TcpConnection::TcpConnection(Socket *sock) :
    Connection(sock){
    _socket = sock;
    _readWatermark = 0;
}


ll::TcpConnection::~TcpConnection(){

}

int
ll::TcpConnection::OnWrite(Context *ctx) {
    int ret;
    ret = _writeBuffer.WriteToFD(_socket->fd);

    if (ret < 0 && errno == EAGAIN) {
        ctx->poller->AddEvent(EVENT_WRITE, this);
    }

    return ret;
}

int 
ll::TcpConnection::Close(Context *ctx) {
    ctx->poller->DelEvent(EVENT_ALL, this);
    ctx->poller->CancelTimeout(this);

    _socket = NULL;
    delete _socket;

    return 0;
}

int
ll::TcpConnection::OnRead(Context *ctx) {
    DLOG(INFO) << "OnRead";
    int ret;
    
    ret = _readBuffer.ReadFromFD(_socket->fd);
    
    if (ret < 0){
        if (errno == EAGAIN) {
            ctx->poller->AddEvent(EVENT_READ, this);
        } else {
            PLOG(INFO) << "onRead error";
            //            this->OnClose(ctx);
            Close(ctx);
        }
    } else if (ret == 0) {
        PLOG(INFO) << "remote closed";
        this->OnClose(ctx);
        Close(ctx);

    } else {
        if (_readBuffer.GetReadableLen() >= _readWatermark) {
            ret = this->OnNewMessage(ctx);
            if (ret >= 0) {
                ctx->poller->AddEvent(EVENT_READ, this);
            } else {
                ctx->poller->DelEvent(EVENT_READ, this);
            }
        }
    }

    return 0;
}


ssize_t
ll::TcpConnection::Recv(Context *ctx, std::string *msg) {
    return _readBuffer.WriteToString(msg);
}


ssize_t
ll::TcpConnection::Send(Context *ctx, const std::string &msg) {
    int ret;
    ret = write(_socket->fd, msg.c_str(), msg.length());
        
    if (ret < 0 && errno == EAGAIN) {
        _writeBuffer.ReadFromString(msg);
        ctx->poller->AddEvent(EVENT_WRITE, this);
    } else {
        DLOG(INFO) << "Write To Server : " << ret << "bytes";
    }
}
