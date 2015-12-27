#ifndef EVENT_H_
#define EVENT_H_

#include <map>
#include <exception>
#include <glog/logging.h>

#include "socket.h"
namespace ll{
    const int EVENT_READ = 0x1;
    const int EVENT_WRITE = 0x2;
    const int EVENT_HUP = 0x4;
    const int EVENT_ONESHOT = 0x100;
    const int EVENT_TIMEOUT = 0x10;
    const int EVENT_ALL = EVENT_READ | EVENT_WRITE | EVENT_ONESHOT;
    const uint64_t EVENT_NO_DEADLINE = 0;

    class Poller;
    class Acceptor;
    class Event;
    class Connection;

    typedef struct {
        Poller *poller;
        Event  *event;
        Socket *socket;

    } Context;

    
    class Event {
    public:
        Event(){
            what = 0;
            deadline = EVENT_NO_DEADLINE;
        }
        int what;
        uint64_t deadline;
        virtual int FD() = 0;
        virtual int OnRead(Context *ctx) {DLOG(INFO) << "EventRead";}
        virtual int OnWrite(Context *ctx) {DLOG(INFO) << "EventWrite";}
        virtual int OnHup(Context *ctx) {DLOG(INFO) << "EventHup";}
        virtual int OnUrgent(Context *ctx) {DLOG(INFO) << "EventUrgent";}
        virtual int OnError(Context *ctx) {DLOG(INFO) << "EventError";}
        virtual int OnTimeout(Context *ctx) {DLOG(INFO) << "EventTimeout";}
    };
   
    template <typename T>
    class Listener : public Event {
    public:
        Listener(Socket *listenSocket);
        virtual ~Listener();

        virtual int FD();
        virtual int OnRead(Context *ctx);

    protected:
        Socket *_socket;
    };


    class EventException : public std::exception {
    public:
        EventException(const std::string &msg) {_msg = msg;}
        virtual ~EventException() throw() {}
        
        virtual const char * what() const throw() {
            return _msg.c_str();
        }

    private:
        std::string _msg;
    };


    class Poller {
    public:
        Poller();
        virtual ~Poller();

        virtual int AddEvent(int what, Event *ev, int timeout=0,int rewrite=0) = 0;
        virtual int DelEvent(int what, Event *ev) = 0;
        virtual int CancelTimeout(Event *ev) = 0;
        virtual int Loop() = 0;
    };


    typedef std::multimap<uint64_t, Event *> TimeoutMap;
    typedef std::multimap<uint64_t, Event *>::iterator TimeoutMapIter;
    class Epoller : public Poller{
    public:
        Epoller(int max_events = 1024);
        virtual ~Epoller();

        int AddEvent(int what, Event *ev, int timeout=0, int rewrite=0);
        int CancelTimeout(Event *ev);
        int DelEvent(int what, Event *ev);
        int Loop();
    private:
        uint64_t _now();

        int _stop;
        struct epoll_event *_events;
        Context *_contexts;
        int _maxEvents, _fd;
        std::map<int, Event *> _eventMap;
        TimeoutMap _timeoutMap;
    };


    class Connection : public Event {
    public:
        Connection(Socket *sock);
        virtual ~Connection();

        virtual int FD() = 0;
      
        virtual int OnRead(Context *ctx);
        virtual int OnWrite(Context *ctx);
        virtual int OnHup(Context *ctx);
        virtual int OnUrgent(Context *ctx);

    protected:
        Connection();
    };
};

#include "listener.hpp"
#endif
