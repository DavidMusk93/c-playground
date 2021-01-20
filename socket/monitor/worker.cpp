#include <fcntl.h>

#include "worker.h"
#include "common.h"
#include "message.h"
#include "timer.h"

using namespace std::placeholders;

static char ipc[64];
static char ipc_notifier[64];
static bool valid_heartbeat_handler = true;

static inline void Touch() {
    int fd = open(ipc_notifier, O_CREAT);
    close(fd);
}

static void Work(int fd) {
    char buf[1024];
    int nr = read(fd, buf, sizeof(buf));
    if (nr > 0) {
        buf[nr] = 0;
        FUNCLOG("recv '%s'", buf);
        write(fd, buf, nr);
    }
}

static void OnRecvFd(int fd, sun::Worker *handler) {
    if (handler->checkCred(fd)) {
        int sock = sun::io::RecvFd(fd);
        if (sock != -1) {
            auto ptr = &handler->state();
            handler->pollInstance().registerEntry(sock, EPOLLRDHUP | EPOLLIN, &Work, [ptr](int) { // touch ipc on close
                Touch();
                *ptr = sun::Worker::State::IDLE;
            });
            remove(ipc_notifier);
            handler->state() = sun::Worker::State::BUSY;
        }
    } else {
        FUNCLOG("invalid peer");
        close(fd);
    }
}

static void Accept(int fd, sun::Worker *handler) {
    int sock;
    ERRRET((sock = accept(fd, nullptr, nullptr)) == -1, , , 1, "accept");
    LOGINFO("new Connection %d", sock);
    handler->pollInstance().registerEntry(sock, EPOLLRDHUP | EPOLLIN, std::bind(&OnRecvFd, _1, handler), {});
}

static void SendPing(int fd) {
    FUNCLOG("");
    sun::Message msg{};
    msg.type = sun::MessageType::PING;
    msg.pid = sun::utility::getpid();
    msg.ping.timestamp = sun::utility::Milliseconds();
    write(fd, &msg, sizeof(msg));
}

static void RecvPong(int fd) {
    sun::Message msg{};
    read(fd, &msg, sizeof(msg));
    FUNCLOG("PONG %d,%f", msg.pid, msg.pong.timestamp);
}

namespace sun {
    Worker::Worker() : state_(State::STARTUP), pid_(-1) {
        sprintf(ipc, WORKER_IPC_PATTERN, utility::getpid());
        sprintf(ipc_notifier, WORKDIR "/%s", ipc);
        io::UnixServer unixServer(ipc);
        pollInstance().registerEntry(unixServer.transferOwnership(), EPOLLIN, std::bind(&Accept, _1, this), {});
        cleanup_ = Defer([this] {
            state_ = State::FINISH;
            remove(ipc);
            remove(ipc_notifier);
        });
    }

    bool Worker::startHeartBeat() {
        Touch();
        if (pid_ != -1) {
            char ipc[64];
            sprintf(ipc, COORDINATOR_IPC_PATTERN, pid_);
            io::UnixClient unixClient(ipc);
            int client_handler = (int) unixClient;
            pollInstance().registerEntry(unixClient.transferOwnership(), EPOLLIN | EPOLLRDHUP, &RecvPong,
                                         [](int fd) { valid_heartbeat_handler = false; });
            Timer::Config config(5);
            Timer timer(config);
            pollInstance().registerEntry(timer.transferOwnership(), EPOLLIN, [client_handler](int fd) {
                Timer::OnTimeout(fd);
                if (valid_heartbeat_handler) {
                    SendPing(client_handler);
                }
            });
            return true;
        }
        return false;
    }

    bool Worker::checkCred(int fd) const {
        if (config_.check_cred) {
            struct ucred cred{};
            socklen_t len = sizeof(cred);
            ERRRET(getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &cred, &len) == -1, false, , 1, "getsockopt#SO_PEERCRED");
            LOGINFO("peer pid=%d,uid=%d,gid=%d", (int) cred.pid, (int) cred.uid, (int) cred.gid);
            return cred.pid == pid_;
        }
        return true;
    }
}