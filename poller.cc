#include <map>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <glog/logging.h>
#include <sys/time.h>
#include "event.h"

using namespace ll;

#ifdef LL_DEBUG
std::string __dumpEvent(int what) {
    std::stringstream ss;

    ss << "Event: "<< what << "[";
#define CHECK_EV(s) if (what & (s)) ss << #s "|"
    CHECK_EV(EPOLLIN);
    CHECK_EV(EPOLLOUT);
    CHECK_EV(EPOLLPRI);
    CHECK_EV(EPOLLERR);
    CHECK_EV(EPOLLHUP);

    ss << "]";
    return ss.str();
}
#endif

Poller::Poller() {
    
}


Poller::~Poller(){

}


Epoller::Epoller(int maxEvents){
    _events = (struct epoll_event *)malloc(sizeof (struct epoll_event) * maxEvents);
    _contexts = (Context *)malloc(sizeof(Context) * maxEvents);
    _maxEvents = maxEvents;

    _fd = epoll_create(1024);
    _stop = 0;
    if (_fd < 0) {
        PLOG(INFO) << "epoll create";
        throw new EventException("create epoller error");
    }
    
    DLOG(INFO) << "create new epoll fd[" << _fd << "]";
}


Epoller::~Epoller(){
    if (_fd > 0) close(_fd);
    if (_contexts) free(_contexts);
    if (_events) free(_events);
}


int
Epoller::AddEvent(int event, Event *ev, int timeout, int rewrite) {
    int ret;
    int op = 0;
    struct epoll_event epev;

    std::map<int, Event *>::const_iterator iter =  _eventMap.find(ev->FD());

    if (rewrite) {
        ev->what = 0;
    }

    if (event & EVENT_READ) {
        ev->what |= EPOLLIN;
    }
    if (event & EVENT_WRITE) {
        ev->what |= EPOLLOUT;
    }

    
    if (iter != _eventMap.end()) {
        op = EPOLL_CTL_MOD;
        DLOG(INFO) << "modify event";
    } else {
        op = EPOLL_CTL_ADD;
        DLOG(INFO) << "add event";
    }

    if (ev->what != 0) {
        epev.events = ev->what;
        epev.data.ptr = ev;

        ret = epoll_ctl(_fd, op, ev->FD(), &epev);
        if (ret < 0) {
            PLOG(INFO) << "epoll ctl add fd [" << ev->FD() << "] error";
            return -1;
        } 
        _eventMap[ev->FD()] = ev;

        DLOG (INFO) <<  "modify event [" << ev->FD() << "] to " << ev->what <<
            " epoll success";
    }


    if (timeout > 0) {
        ev->deadline = _now() + timeout;
        DLOG(INFO) << "add event deadline: " << ev->deadline;
        _timeoutMap.insert(std::make_pair(ev->deadline, ev));
    }

    return 0;
}

int
Epoller::DelEvent(int event, Event *ev) {
    std::map<int, Event *>::iterator iter =  _eventMap.find(ev->FD());
    int ret, op;
    struct epoll_event *p = NULL, epev;

    if (event & EVENT_READ) {
        ev->what &= ~EPOLLIN;
    }
    if (event & EVENT_WRITE) {
        ev->what &= ~EPOLLOUT;
    }

    if (iter != _eventMap.end()) {
        if (ev->what == 0) {
            op = EPOLL_CTL_DEL;
        } else {
            op = EPOLL_CTL_MOD;

            epev.events = ev->what;
            epev.data.ptr = ev;

            p = &epev;
        }

        ret =  epoll_ctl(_fd, op, ev->FD(), p);
        if (ret < 0) {
            PLOG(INFO) <<  "epoll ctl del";
            return -1;
        } else {
            if (op == EPOLL_CTL_DEL) {
                _eventMap.erase(ev->FD());
            }
        }
        
        return 0;
    }

    return -1;
}


int Epoller::CancelTimeout(Event *ev){
    std::pair<TimeoutMapIter, TimeoutMapIter> iter 
        = _timeoutMap.equal_range(ev->deadline);

    TimeoutMapIter eiter;
    for (eiter = iter.first; eiter != iter.second; eiter++) {
        if (eiter->second == ev) {
            _timeoutMap.erase(eiter);
            break;
        }
    }

    return 0;
}


uint64_t
Epoller::_now() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t) tv.tv_sec * 1000 + tv.tv_usec/1000;
}


int 
Epoller::Loop() {
    int nevents, i;
    int timeout;
    uint64_t after_loop;

    while (!_stop) {
        std::multimap<uint64_t, Event *>::iterator iter = _timeoutMap.lower_bound(0);
        if (iter == _timeoutMap.end()) {
            timeout = -1;
        } else {
            timeout = (int) (iter->first - _now());
        }


        DLOG(INFO) << "Epoll entering loop with: " << timeout;
        nevents = epoll_wait(_fd, _events, _maxEvents, timeout);
        after_loop = _now();
        
        if (nevents < 0){
            PLOG(INFO) << "wait event error";
        } else {
            DLOG(INFO) << "Got " << nevents << " events";
            for (i = 0; i < nevents; i++) {

                Context *ctx = _contexts + i;
                ctx->poller = this;
                ctx->event = (Event *)_events[i].data.ptr;
#ifdef LL_DEBUG
                DLOG(INFO) << "Event " << i << "\t" << ctx->event->FD()
                           << "," << __dumpEvent(_events[i].events);
#endif

                if (_events[i].events & EPOLLIN || _events[i].events & EPOLLWRNORM){
                    ctx->event->OnRead(ctx);
                }
                if (_events[i].events & EPOLLOUT) {
                    ctx->event->OnWrite(ctx);
                }
                if (_events[i].events & EPOLLHUP) {
                    ctx->event->OnHup(ctx);
                }
                if (_events[i].events & EPOLLERR) {
                    ctx->event->OnError(ctx);
                }
                if (_events[i].events & EPOLLPRI) {
                    ctx->event->OnUrgent(ctx);
                }

                /*
                if (ctx->event->deadline != EVENT_NO_DEADLINE) {
                    DLOG(INFO) << "canceled deadline";
                    CancelTimeout(ctx->event);
                    }*/
            }
        }

        iter = _timeoutMap.lower_bound(0);
        std::multimap<uint64_t, Event *>::iterator end_iter = _timeoutMap.upper_bound(after_loop);
        std::multimap<uint64_t, Event *>::iterator begin_iter = _timeoutMap.lower_bound(0);

        int i = 0;
        for (iter = begin_iter; iter != end_iter; iter++) {
            Context *ctx = _contexts + i++;
            ctx->poller = this;
            ctx->event = iter->second;
        }

        _timeoutMap.erase(begin_iter, end_iter);
        for (int j = 0; j < i; j++) {
            _contexts[j].event->OnTimeout(&_contexts[j]);
        }
    }
}
