#include <map>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <glog/logging.h>

#include "event.h"

using namespace ll;

Connection::Connection(){
}

Connection::Connection(Socket *sock){
}

Connection::~Connection() {
}


int
Connection::OnRead(Context *ctx) {
    DLOG(INFO) << "OnRead";
}

int
Connection::OnWrite(Context *ctx) {
    DLOG(INFO) << "OnWrite";
}

int
Connection::OnHup(Context *ctx) {
    DLOG(INFO) << "OnHup";
}

int
Connection::OnUrgent(Context *ctx) {
    DLOG(INFO) << "OnUrgent";
}
