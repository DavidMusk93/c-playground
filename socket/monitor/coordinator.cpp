#include <mutex>
#include <fstream>
#include <unordered_map>

#include <sys/stat.h>
#include <sys/un.h>
#include <signal.h>

#include "coordinator.h"
#include "time_wheel.h"
#include "timer.h"
#include "message.h"
#include "util.h"

using namespace std::placeholders;

std::mutex mtx;
#define LOCKGUARD(x) std::lock_guard<std::mutex> __lk(x)
#define IDLEWORKERS(ptr) (ptr)->workers(sun::Coordinator::WorkerType::IDLE)
#define BUSYWORKERS(ptr) (ptr)->workers(sun::Coordinator::WorkerType::BUSY)

static void OnStart() {
    mkdir(WORKDIR, 0660);
    std::ofstream ofs(COORDINATOR_PIDFILE, std::ios::trunc);
    ofs << sun::utility::GetPid();
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
            if (sun::utility::ValidProcess(it->pid) && sun::utility::ValidFd(it->handler)) {
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

struct WorkerHeartBeatInfo {
    int pid{};
    int handler{-1};
    double timestamp{};

    static std::shared_ptr<WorkerHeartBeatInfo> Create(int handler);
};

std::shared_ptr<WorkerHeartBeatInfo> WorkerHeartBeatInfo::Create(int handler) {
    auto rval = std::make_shared<WorkerHeartBeatInfo>();
    rval->handler = handler;
    return rval;
}

static sun::TimeWheel<WorkerHeartBeatInfo, 10> timeWheel;
static std::unordered_map<int, std::shared_ptr<WorkerHeartBeatInfo>> heartBeatInfo;

static void OnWheelTick(int fd, sun::Coordinator *handler) {
    sun::Timer::OnTimeout(fd);
    auto slot = timeWheel.tick();
    for (auto &i:slot) {
        if (i.use_count() == 2) {
            FUNCLOG("%d#%d timeout, kick it out", i->pid, i->handler);
            handler->pollInstance().remove(i->handler);
            heartBeatInfo.erase(i->handler);
        }
    }
}

static void OnRecvHeartBeat(int fd, sun::Coordinator *handler) {
    sun::Message msg{};
    recv(fd, &msg, sizeof(msg), MSG_WAITALL);
    if (msg.type == sun::MessageType::PING) {
        FUNCLOG("PING %d,%f", msg.pid, msg.ping.timestamp);
        auto &info = heartBeatInfo[fd];
        info->pid = msg.pid;
        info->timestamp = sun::utility::Milliseconds();
        timeWheel.current().push_back(info);
        msg.type = sun::MessageType::PONG;
        msg.pid = sun::utility::GetPid();
        msg.pong.timestamp = info->timestamp;
        write(fd, &msg, sizeof(msg));
    } else {
        FUNCLOG("unknown Message type");
    }
}

static void AcceptWorkerConnection(int fd, sun::Coordinator *handler) {
    int sock;
    ERRRET((sock = accept(fd, nullptr, nullptr)) == -1, , , 1, "accept");
    LOGINFO("new Worker Heart Beat connection %d", sock);
    auto info = WorkerHeartBeatInfo::Create(sock);
    heartBeatInfo[sock] = info;
    timeWheel.current().push_back(info);
    handler->pollInstance().registerEntry(sock, EPOLLRDHUP | EPOLLIN, std::bind(&OnRecvHeartBeat, _1, handler));
}

namespace sun {
    Coordinator::Coordinator() : ipc_{} {
        OnStart();
        io::TcpipServer tcpipServer(config_.port);
        pollInstance().registerEntry(tcpipServer.transferOwnership(), EPOLLIN,
                                     std::bind(&Accept, _1, this));
        fw_.registerCallback(io::FileWatcher::EventType::CREATE, std::bind(&OnIpcFileCreate, _1, this))
                .watchPath(WORKDIR);
        sun::Timer::Config config(1);
        sun::Timer timer(config);
        pollInstance().registerEntry(timer.transferOwnership(), EPOLLIN, std::bind(&OnWheelTick, _1, this));
        sprintf(ipc_, COORDINATOR_IPC_PATTERN, utility::GetPid());
        io::UnixServer unixServer(ipc_);
        pollInstance().registerEntry(unixServer.transferOwnership(), EPOLLIN,
                                     std::bind(&AcceptWorkerConnection, _1, this));
        cleanup_ = Defer([this] {
            remove(COORDINATOR_PIDFILE);
            remove(ipc_);
        });
    }

    Coordinator::~Coordinator() {
        fw_.stop();
        for (auto &worker:busy_workers_) {
            utility::Close(worker.handler);
        }
        for (auto &worker:idle_workers_) {
            utility::Close(worker.handler);
        }
    }

    void Coordinator::loop() { // block waiting
        fw_.start();
        pollInstance().loop();
    }
}