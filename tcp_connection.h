#ifndef TCPCONNECTION_H_
#define TCPCONNECTION_H_

#include "buffer.h"
#include "event.h"

namespace ll{
    class TcpConnection : public Connection {
    public:
        TcpConnection(Socket *sock);
        virtual ~TcpConnection();

        virtual int FD(){
            return _socket->fd;
        }

        virtual int SetReadWatermark(size_t mark) {
            _readWatermark = mark;
        }
        
        virtual int Close(Context *ctx);
        virtual int OnCreated(Context *ctx) = 0;
        virtual int OnNewMessage(Context *ctx) = 0;
        virtual int OnClose(Context *ctx) = 0;
        //        virtual int OnBrokenPipe(Context *ctx) = 0;

        virtual int OnWrite(Context *ctx);
        virtual int OnRead(Context *ctx);
        //        virtual int OnHup(Context *ctx);
        //virtual int OnError(Context *ctx);

        ssize_t Recv(Context *ctx, void *buf, ssize_t len);
        ssize_t Recv(Context *ctx, std::string *msg);
        ssize_t Send(Context *ctx, const void *buf, ssize_t len);
        ssize_t Send(Context *ctx, const std::string &msg);
    protected:
        Buffer _readBuffer, _writeBuffer;
        size_t _readWatermark;
        Socket *_socket;
    };
};

#endif
