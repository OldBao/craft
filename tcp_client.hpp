#include "tcp_connection.h"

template <typename T>
ll::TcpClient<T>::TcpClient() {
}

template <typename T>
ll::TcpClient<T>::~TcpClient(){

}

template <typename T> int 
ll::TcpClient<T>::OnConnected(Context *ctx) {
    TcpConnection *t = new T(ctx->socket);
    ctx->poller->AddEvent(EVENT_READ, t);
    t->OnCreated(ctx);
}


template <typename T> int
ll::TcpClient<T>::OnConnectFail(Context *ctx) {
    
}
