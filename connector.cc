#include "connector.h"
using namespace ll;

TcpConnector::TcpConnector(int timeout, int retry) {
    _timeout = timeout;
    _retries = retry;
    _state = DISCONN;
    _try = 1;
}

TcpConnector::~TcpConnector(){

}

int 
TcpConnector::Disconnect(Poller *poller){
    delete _socket;

    _socket = NULL;
    _state = DISCONN;

    poller->DelEvent(EVENT_ALL, this);
    poller->CancelTimeout(this);

    _try = 1;
    return 0;
}

int
TcpConnector::ConnectTo(Poller *poller, const std::string &addr, uint16_t port) {
    int ret;

    if (_state == DISCONN) {
        _addr = addr;
        _port = port;
        _socket = new Socket(AF_INET, SOCK_STREAM, 0);
        _socket->SetBlocking(0);

        ret = _socket->Connect(addr, port);
        if (ret != 0 && errno == EINPROGRESS) {
            DLOG(INFO) << "CONNECTING";
            _state = CONNECTING;
            poller->AddEvent(EVENT_WRITE, this, _timeout);

        } else {
            poller->CancelTimeout(this);
            _state = CONNECTED;
        }
        return 0;
    }
    
    assert(0);
}

int
TcpConnector::OnTimeout(Context *ctx) {
    switch (_state) {
    case CONNECTING:
        // connect timeout

        if (0 == _handleFails(ctx)) {
            _state = DISCONN;
            ConnectTo(ctx->poller, _addr, _port);
        }

        break;


    case DISCONN:
        // timeout after waiting
        ConnectTo(ctx->poller, _addr, _port);
        break;

    default:
        //other state should not appears hear
        assert(0);
    }
    return 0;
}


int
TcpConnector::OnWrite(Context *ctx) {
    if (_state == CONNECTING) {
        _state = CONNECTED;
        ctx->poller->CancelTimeout(this);        
        ctx->poller->DelEvent(EVENT_ALL, this);
        ctx->socket = _socket;
        this->OnConnected(ctx);
    }
}


int
TcpConnector::OnHup(Context *ctx) {
    LOG(INFO) << "OnHup";

    if (0 == _handleFails(ctx)) {
        _state = DISCONN;
        ctx->poller->AddEvent(EVENT_TIMEOUT, this, _timeout);
    }

    return 0;
}


int
TcpConnector::OnError(Context *ctx) {
    LOG(INFO) << "OnError";
    //    return _retry(ctx);
}


int
TcpConnector::_handleFails(Context *ctx){
    if (_try >= _retries) {
        LOG(INFO) << "fail after retries";
        _state = DISCONN;
        this->OnConnectFail(ctx);

        ctx->poller->DelEvent(EVENT_ALL, this);
        ctx->poller->CancelTimeout(this);
        delete _socket;
        return -1;
    }

    LOG(INFO) << "Connect to " << _addr << ":" << _port << " error, try (" <<
        _try << "/" << _retries << ")";

    _try++;
    ctx->poller->DelEvent(EVENT_ALL, this);
    ctx->poller->CancelTimeout(this);
    delete _socket;

    return 0;
}
