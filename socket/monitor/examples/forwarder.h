#ifndef MONITOR_FORWARDER_H
#define MONITOR_FORWARDER_H

#include <unordered_map>
#include <list>

#include "loop.h"

#define MEMBER_OFFSET(t, x) (int)(&((t*)0)->x)
#define PAYLOADHEADER_LENGTH 40

#define LIST_FOLLOWERCONTEXT std::list<FollowerContext>
#define ITERATOR_FOLLOEWRCONTEXT LIST_FOLLOWERCONTEXT::iterator

#define CLIENTINFO PAYLOADHEADER_partial
#define LIST_CLIENTINFO std::list<CLIENTINFO>
#define ITERATOR_CLIENTINFO LIST_CLIENTINFO::iterator

#define FORWARDER_IPCFILE "/tmp/forwarder.ipc"
#define FORWARDER_NOTIFYSIGNAL SIGRTMIN+2
#define FORWARDER_SERVICEPORT 23402

struct PAYLOADHEADER_partial {
    int fd; // operation id (short) & dcs version (short) is trivial
    int dialogue;
};

struct FollowerContext {
    int pid;
    int fd; // the POLL would close it
    int dialogue;
    double timestamp;

//    ~FollowerContext() {
//        if (fd != -1) {
//            sun::util::Close(fd);
//        }
//    }

    void publish(ITERATOR_CLIENTINFO it) {
        publish(*it);
    }

    void publish(CLIENTINFO &ci);

    FollowerContext() : pid(0), fd(-1), dialogue(-1), timestamp(0) {}
};

namespace sun {
    class Forwarder : public Loop {
    public:
        Forwarder();

        LIST_CLIENTINFO list_client;
        LIST_FOLLOWERCONTEXT list_follower;
        std::unordered_map<int/*pid*/, ITERATOR_FOLLOEWRCONTEXT> map_follower; // auxiliary data to boost search
    };
}

#endif //MONITOR_FORWARDER_H
