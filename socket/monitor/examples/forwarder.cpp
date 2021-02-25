#include "forwarder.h"

#include <vector>

#include <assert.h>
#include <sys/signalfd.h>
#include <poll.h>

#include "sock.h"
#include "callback.h"
#include "signal_fd.h"

#define _GETFOLLOWER(x) handler->x##_follower
#define LISTFOLLOWER _GETFOLLOWER(list)
#define MAPFOLLOWER _GETFOLLOWER(map)
#define LISTCLIENT handler->list_client

#define FORWARDER sun::Forwarder

using namespace std::placeholders;

#define DECLARECALLBACK(name, fd, context) static void name(int fd,FORWARDER*context)
#define IMPLCALLBACK(name) DECLARECALLBACK(name,fd,handler)
#define BINDCALLBACK(name, context) std::bind(&name,_1,context)

DECLARECALLBACK(accept_client, ,);

DECLARECALLBACK(accept_follower, ,);

DECLARECALLBACK(notify_connecting, ,);

DECLARECALLBACK(peek_header, ,);

void FollowerContext::publish(ITERATOR_CLIENTINFO it) {
    // @TODO make robust
    sun::io::SendFd(fd, it->fd);
    dialogue = -1;
    timestamp = sun::util::Milliseconds();
}

namespace sun {
    Forwarder::Forwarder() : error_count(0) {
        io::UnixServer unixServer(FORWARDER_IPCFILE);
        pollInstance().registerEntry(unixServer.transferOwnership(), EPOLLIN, BINDCALLBACK(accept_follower, this));
        SignalFd signalFd(FORWARDER_NOTIFYSIGNAL);
        pollInstance().registerEntry(signalFd.transferOwnership(), EPOLLIN, BINDCALLBACK(notify_connecting, this));
        io::TcpipServer tcpipServer(FORWARDER_SERVICEPORT);
        if (tcpipServer.valid()) {
            pollInstance().registerEntry(tcpipServer.transferOwnership(), EPOLLIN, BINDCALLBACK(accept_client, this));
        } else {
            ++error_count;
        }
    }
}

IMPLCALLBACK(accept_client) {
    sun::OnTcpipAccept(fd, &handler->pollInstance(), BINDCALLBACK(peek_header, handler));
}

IMPLCALLBACK(accept_follower) {
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

IMPLCALLBACK(notify_connecting) {
    struct signalfd_siginfo si{};
    read(fd, &si, sizeof(struct signalfd_siginfo));
    FUNCLOG("dialogue,%d,%d,%d", si.ssi_int, si.ssi_signo, si.ssi_pid);
    auto p = MAPFOLLOWER.find(si.ssi_pid);
    if (p != MAPFOLLOWER.end()) { // a registered follower
        auto dialogue = p->second->dialogue = si.ssi_int;
        std::vector<ITERATOR_CLIENTINFO> invalid;
        auto current = sun::util::Milliseconds();
        for (auto it = LISTCLIENT.begin(), end = LISTCLIENT.end(); it != end; ++it) {
            if (current - it->timestamp > CLIENTINFO_EXPIREDTHRESHOLD) {
                invalid.push_back(it);
                continue;
            }
            // Case 1, connection come first
            if (it->dialogue == dialogue) {
                p->second->publish(it);
                sun::util::Close(it->fd);
                LISTCLIENT.erase(it);
                break;
            }
        }
        // Avoid memory leak
        for (auto &it:invalid) {
            LISTCLIENT.erase(it);
        }
    }
}

IMPLCALLBACK(peek_header) {
    sun::io::Poll::Entry entry;
    LISTCLIENT.push_front({});
    auto it = LISTCLIENT.begin();
    int nr = recv(fd, &*it, PAYLOADHEADER_PARTIALLENGTH, MSG_PEEK | MSG_WAITALL);
    assert(nr == PAYLOADHEADER_PARTIALLENGTH);
    it->timestamp = sun::util::Milliseconds();
    FUNCLOG("#%d is ready,%d", fd, it->dialogue);
    handler->pollInstance().remove(fd, &entry);
    it->fd = int(entry);
    for (auto &p:MAPFOLLOWER) {
        // Case 2, signal come first
        if (p.second->dialogue == it->dialogue) {
            p.second->publish(it);
            LISTCLIENT.pop_front();
            return;
        }
    }
    entry.giveupOwnership();
}

#include <pwd.h>

static bool setguid(const char *username) {
#define DEFAULTUSERNAME "trafodion"
    auto *pwd = getpwnam(username ?: DEFAULTUSERNAME);
    if (pwd) {
        return setgid(pwd->pw_gid) == 0/*set gid first*/ && setuid(pwd->pw_uid) == 0;
    }
    return false;
}

#include "test.h"
#include "status.h"

DECLAREPOLLSIGNALHANDLER(g_handler, sig_handler);

#define PROGRAMNAME "forwarder"
#define TMPDIR "/tmp"
#define LOCKFILE TMPDIR "/" PROGRAMNAME ".lock"
#define LOGFILE TMPDIR "/" PROGRAMNAME ".log"

MAIN_EX(argc, argv) {
    sun::FileLock fl(LOCKFILE, sun::RECORD);
    if (fl.setType(F_WRLCK).lock() == -1) {
        return kConflict;
    }
    using namespace sun::util;
    RedirectOutput(LOGFILE);
    Daemon::Task submain = [&fl](Daemon::Context *ctxp) {
        fl.cleanup();
        do {
            int rval, status{0};
            struct pollfd pfd{.fd=ctxp->fd, .events=POLLIN};
            POLL(rval, poll, &pfd, 1, -1);
            if (rval == 1 && read(pfd.fd, &status, sizeof(status)) == sizeof(status) && status == kConflict) {
                LOGERROR("[%s]fatal error,quit", ctxp->name.c_str());
                return;
            }
        } while (0);
        int duration = 5;
        LOGINFO("[%s] master dead,revoke it(after %ds)", ctxp->name.c_str(), duration);
        sleep(duration); // duration to regret
        execl(ctxp->exe.c_str(), ctxp->exe.c_str(), 0);
    };
    int notifier;
    const char *username = argv[1];
    Daemon::Task onfork = [&notifier, username](Daemon::Context *ctxp) {
        notifier = ctxp->fd;
        LOGINFO("set {g,u}id %s", setguid(username) ? "SUCCESS" : "FAILURE");
    };
    Daemon daemon(PROGRAMNAME ".daemon", submain, onfork);
    daemon.run();
    sun::Forwarder forwarder;
    if (forwarder.error_count) {
        int code = kConflict;
        write(notifier, &code, sizeof(code));
        return code;
    }
    g_handler = &forwarder.pollInstance();
    INSTALLSIGINTHANDLER(&sig_handler);
    forwarder.loop(); // LOOP occupies the main thread (process)
    close(notifier);
    return 0;
}