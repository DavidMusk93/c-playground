#include <fcntl.h>

#include "worker.h"
#include "common.h"

using namespace std::placeholders;

static char ipc[64];
static char ipc_notifier[64];

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
}

static void Accept(int fd, sun::Worker *handler) {
    int sock;
    ERRRET((sock = accept(fd, nullptr, nullptr)) == -1, , , 1, "accept");
    LOGINFO("new Connection %d", sock);
    handler->pollInstance().registerEntry(sock, EPOLLRDHUP | EPOLLIN, std::bind(&OnRecvFd, _1, handler), {});
}

namespace sun {
    Worker::Worker() : state_(State::STARTUP) {
        sprintf(ipc, WORKER_IPC_PATTERN, utility::getpid());
        sprintf(ipc_notifier, WORKDIR "/%s", ipc);
        io::UnixServer unixServer(ipc);
        pollInstance().registerEntry(unixServer.transferOwnership(), EPOLLIN, std::bind(&Accept, _1, this), {});
        cleanup_ = Defer([this] {
            state_ = State::FINISH;
            remove(ipc);
            remove(ipc_notifier);
        });
        Touch();
    }
}