//
// Created by esgyn on 1/15/2021.
//

#include <mutex>
#include <fstream>

#include <sys/stat.h>
#include <sys/un.h>
#include <signal.h>

#include "coordinator.h"

std::mutex mtx;
#define LOCKGUARD(x) std::lock_guard<std::mutex> __lk(x)
#define IDLEWORKERS(ptr) (ptr)->workers(sun::Coordinator::WorkerType::IDLE)
#define BUSYWORKERS(ptr) (ptr)->workers(sun::Coordinator::WorkerType::BUSY)

static void OnStart() {
    mkdir(WORKDIR, 0660);
    std::ofstream ofs(COORDINATOR_PIDFILE, std::ios::trunc);
    ofs << sun::utility::getpid();
}

static void OnIpcFileCreate(struct inotify_event *ev, sun::Coordinator *handler) {
    FUNCLOG("%s", ev->name);
    int pid{};
    sscanf(ev->name, WORKER_IPC_PATTERN, &pid);
    if (pid) {
        mtx.lock();
        auto busy_workers = BUSYWORKERS(handler);
        mtx.unlock();
        sun::Coordinator::Iterator it;
        for (it = busy_workers.begin(); it != busy_workers.end(); ++it) {
            if (it->pid == pid) {
                break;
            }
        }
        if (it != busy_workers.end()) { // busy worker to idle
            busy_workers.erase(it);
            LOCKGUARD(mtx);
            IDLEWORKERS(handler).push_back(*it);
            BUSYWORKERS(handler).swap(busy_workers);
        } else { // new worker
            sun::Coordinator::WorkerMeta meta;
            meta.pid = pid;
            meta.handler = socket(AF_UNIX, SOCK_STREAM, 0);
            struct sockaddr_un sa{};
            sa.sun_family = AF_UNIX;
            strcpy(sa.sun_path, ev->name);
            ERRRET(connect(meta.handler, SOCKADDR_EX(sa)) == -1, , , 1, "connect");
            LOCKGUARD(mtx);
            IDLEWORKERS(handler).push_back(meta);
        }
    }
}

//static void OnIpcFileDelete(struct inotify_event *ev, sun::Coordinator *handler) {
//
//}

static inline bool IsValidProcess(int pid) {
    return kill(pid, 0) == 0;
}

static inline bool IsOpenFd(int fd) {
    return fcntl(fd, F_GETFD) != -1;
}

static void Accept(int fd, sun::Coordinator *handler) {
    struct sockaddr_in peer{};
    socklen_t len = sizeof(peer);
    int sock{-1};
    ERRRET((sock = accept4(fd, SOCKADDRPTR(&peer), &len, 0)) == -1, , , 1, "accept4");
    LOGINFO("new Connection " SOCKADDR_FMT, SOCKADDR_OF(peer));
    mtx.lock();
    auto idle_workers = IDLEWORKERS(handler);
    mtx.unlock();
    sun::Coordinator::Iterator it;
    do {
        for (it = idle_workers.begin(); it != idle_workers.end();) {
            if (IsValidProcess(it->pid) && IsOpenFd(it->handler)) {
                break;
            } else {
                LOGINFO("invalid worker %d#%d", it->pid, it->handler);
                it = idle_workers.erase(it);
            }
        }
        if (it == idle_workers.end()) {
            break;
        }
        LOGINFO("send %d to %d#%d", sock, it->pid, it->handler);
        sun::io::SendFd(it->handler, sock);
        idle_workers.erase(it);
        LOCKGUARD(mtx);
        BUSYWORKERS(handler).push_back(*it);
        IDLEWORKERS(handler).swap(idle_workers);
    } while (0);
    if (it == idle_workers.end()) {
        LOGINFO("no idle workers");
    }
    close(sock);
}

namespace sun {
    Coordinator::Coordinator() {
        using namespace std::placeholders;
        OnStart();
        io::TcpipServer tcpipServer(config_.port);
        pollInstance().registerEntry(tcpipServer.transferOwnership(), EPOLLIN,
                                     std::bind(&Accept, _1, this),
                                     {});
        fw_.registerCallback(io::FileWatcher::EventType::CREATE, std::bind(&OnIpcFileCreate, _1, this))
                .watchPath(WORKDIR);
        cleanup_ = Defer([] { remove(COORDINATOR_PIDFILE); });
    }

    Coordinator::~Coordinator() {
        fw_.stop();
        for (auto &worker:busy_workers_) {
            close(worker.handler);
        }
        for (auto &worker:idle_workers_) {
            close(worker.handler);
        }
    }

    void Coordinator::loop() { // block waiting
        fw_.start();
        pollInstance().loop();
    }
}