#include "forwarder.h"

#include <assert.h>
#include <sys/signalfd.h>

#include "sock.h"
#include "callback.h"
#include "signal_fd.h"

#define _GETFOLLOWER(x) handler->x##_follower
#define LISTFOLLOWER _GETFOLLOWER(list)
#define MAPFOLLOWER _GETFOLLOWER(map)
#define LISTCLIENT handler->list_client

#define FORWARDER sun::Forwarder

using namespace std::placeholders;

static void accept_client(int fd, FORWARDER *handler);
static void accept_follower(int fd, FORWARDER *handler);
static void notify_connecting(int fd, FORWARDER *handler);
static void peek_header(int fd, FORWARDER *handler);

void FollowerContext::publish(CLIENTINFO &ci) {
    // @TODO make robust
    if (sun::io::SendFd(fd, ci.fd)) {
//        close(ci.fd); // close by poll entry
        dialogue = -1;
        timestamp = sun::util::Milliseconds();
    }
}

namespace sun {
    Forwarder::Forwarder() {
        io::UnixServer unixServer(FORWARDER_IPCFILE);
        pollInstance().registerEntry(unixServer.transferOwnership(), EPOLLIN, std::bind(&accept_follower, _1, this));
        SignalFd signalFd(FORWARDER_NOTIFYSIGNAL);
        pollInstance().registerEntry(signalFd.transferOwnership(), EPOLLIN, std::bind(&notify_connecting, _1, this));
        io::TcpipServer tcpipServer(FORWARDER_SERVICEPORT);
        pollInstance().registerEntry(tcpipServer.transferOwnership(), EPOLLIN, std::bind(&accept_client, _1, this));
    }
}

static void accept_client(int fd, FORWARDER *handler) {
    sun::OnTcpipAccept(fd, &handler->pollInstance(), std::bind(&peek_header, _1, handler));
}

static void accept_follower(int fd, FORWARDER *handler) {
    static int pid = sun::util::GetPid();
    int sock = accept(fd, nullptr, nullptr);
    ERRRET(sock == -1, , , 1, "accept");
    LOGINFO("new follower %d", sock);
    LISTFOLLOWER.push_front({});
    auto it = LISTFOLLOWER.begin();
    // exchange pid (follower & forwarder)
    read(sock, &it->pid, sizeof(int));
    write(sock, &pid, sizeof(int));
    it->fd = sock;
    it->timestamp = sun::util::Milliseconds();
    MAPFOLLOWER.insert({it->pid, it});
    handler->pollInstance().registerEntry(sock, EPOLLRDHUP, {}, /*follower quit*/[handler](int fd) {
        ITERATOR_FOLLOEWRCONTEXT it, end;
        for (it = LISTFOLLOWER.begin(), end = LISTFOLLOWER.end(); it != end; ++it) {
            if (it->fd == fd) {
                LISTFOLLOWER.erase(it);
                MAPFOLLOWER.erase(it->pid);
                break;
            }
        }
    });
}

static void notify_connecting(int fd, FORWARDER *handler) {
    struct signalfd_siginfo si{};
    read(fd, &si, sizeof(struct signalfd_siginfo));
    FUNCLOG("dialogue,%d,%d,%d", si.ssi_int, si.ssi_signo, si.ssi_pid);
    auto p = MAPFOLLOWER.find(si.ssi_pid);
    if (p != MAPFOLLOWER.end()) { // a registered follower
        auto dialogue = p->second->dialogue = si.ssi_int;
        // Case 1, connection come first
        for (auto it = LISTCLIENT.begin(), end = LISTCLIENT.end(); it != end; ++it) {
            if (it->dialogue == dialogue) {
                p->second->publish(it);
                LISTCLIENT.erase(it);
                break;
            }
        }
    }
}

static void peek_header(int fd, FORWARDER *handler) {
    FUNCLOG("#%d is ready", fd);
    sun::io::Poll::Entry entry;
    LISTCLIENT.push_front({});
    auto it = LISTCLIENT.begin();
    int nr = recv(fd, &*it, sizeof(CLIENTINFO), MSG_PEEK | MSG_WAITALL);
    assert(nr == sizeof(CLIENTINFO));
    handler->pollInstance().remove(fd, &entry);
//    it->fd = entry.transferOwnership(); // make sure unregister correctly
    it->fd = int(entry);
    // Case 2, signal come first
    for (auto &p:MAPFOLLOWER) {
        if (p.second->dialogue == it->dialogue) {
            p.second->publish(it);
            LISTCLIENT.pop_front();
            break;
        }
    }
}

#include "test.h"

DECLAREPOLLSIGNALHANDLER(g_handler, sig_handler);

int main() {
    sun::Forwarder forwarder;
    g_handler = &forwarder.pollInstance();
    INSTALLSIGINTHANDLER(&sig_handler);
    forwarder.loop(); // LOOP occupies the main thread (process)
}