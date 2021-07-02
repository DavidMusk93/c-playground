#include "listener.h"
#include "util.h"
#include "sock.h"

namespace sun {
#define NOW() sun::util::Milliseconds()

    bool Heartbeat::ping(int fd, Reader *r) {
        auto now = NOW();
        if (now - r->lastest_readts < HEARTBEAT_DURATION) { /*no need to ping*/
            return true;
        }
        if (!retry) {
            return false;
        }
        if (lastest_pingts > 0 && (
                rtt < 0 /*first ping, without pong*/||
                now - lastest_pongts > HEARTBEAT_DURATION + 20 * minrtt /*pong timeout*/)) {
            --retry;
            LOGINFO("no pong form peer(%.3f,%.3f,%.3f), remaining retry: %d", rtt, minrtt, maxrtt, retry);
        }
        io::MsgHeartbeat msg(now);
        io::MsgRaw raw(io::MSGPING);
        raw.payload = msg.pack();
        if (raw.write(fd)) {
            lastest_pingts = now;
            return true;
        }
        return false;
    }

    void Heartbeat::reset() {
        lastest_pingts = lastest_pongts = -1;
        rtt = -1;
        minrtt = HEARTBEAT_DURATION;
        maxrtt = -HEARTBEAT_DURATION;
        retry = HEARTBEAT_MAXRETRY;
    }

    void Heartbeat::updateRTT(double d) {
        rtt = d;
        if (minrtt > d) {
            minrtt = d;
        }
        if (maxrtt < d && d < HEARTBEAT_DURATION / 10) {
            maxrtt = d;
        }
        retry = HEARTBEAT_MAXRETRY;
    }

    void Reader::read() {
        io::MsgRaw raw;
        if (raw.read(listener->peerfd)) {
            auto now = NOW();
            io::Unpacker r(raw.payload);
            lastest_readts = now;
            switch (raw.type) {
                case io::MSGREQUEST: {
                    if (msgreq.unpack(&r)) {
                        listener->metrics.ops |= OPMASK;
                        Task t;
                        t.hook.oncall = [](void *arg) { LOGINFO("@WORKER task start(%p)", arg); };
                        t.hook.onreturn = [](const void *arg) { LOGINFO("@WORKER task stop(%p)", arg); };
                        t.fn = Listener::Work;
                        t.arg = listener;
                        LOGINFO("@LISTENER post task");
                        listener->worker.post(t);
                    } else {
                        listener->onUnpackError();
                    }
                    break;
                }
                case io::MSGPONG: {
                    if (msght.unpack(&r)) {
                        listener->heartbeat.lastest_pongts = now;
                        listener->heartbeat.updateRTT(now - msght.timestamp);
                        LOGINFO("pong rrt:%.3fms", listener->heartbeat.rtt);
                    } else {
                        listener->onUnpackError();
                    }
                    break;
                }
                default:
                    LOGERROR("unknown message type: %d", raw.type);
            }
        } else {
            listener->onDisconnect();
        }
    }

    void Writer::pub(io::MsgRaw &&msg) {
        mtx.lock();
        msgs.emplace_back(msg.move());
        mtx.unlock();
    }

    void Writer::write() {
        std::vector<io::MsgRaw> t;
        mtx.lock();
        t.swap(msgs);
        mtx.unlock();
        for (auto &msg:t) {
            if (msg.write(listener->peerfd)) {
                if (msg.type == io::MSGRESPONSE) {
                    auto &ops = listener->metrics.ops;
                    if (ops & OPMASK) {
                        ops &= ~OPMASK;
                        ops++;
                    } else {
                        LOGERROR("request & response do not match");
                    }
                }
            } else {
                listener->onDisconnect();
                break;
            }
        }
    }

    Listener::Listener(short port) : activefd(-1), peerfd(-1), msghub(),
                                     r(this), w(this),
                                     state(State::UNINITIALIZED), metrics() {
        io::TcpipServer server(port);
        if (server.valid()) {
            activefd = server.transferOwnership();
            onIdle();
        }
    }

    Listener::~Listener() {
        if (valid()) {
            msghub.notify(kNotifyLarge);
            worker.stop();
            if (activefd != -1) {
                close(activefd);
            }
        }
    }

    bool Listener::valid() const {
        return state != State::UNINITIALIZED;
    }

    bool Listener::idle() const {
        return state.load(std::memory_order_acquire) == State::IDLE;
    }

    bool Listener::work() const {
        return state.load(std::memory_order_acquire) == State::WORK;
    }

    void Listener::onWork() {
        state.store(State::WORK, std::memory_order_release);
    }

    void Listener::onIdle() {
        state.store(State::IDLE, std::memory_order_acquire);
    }

    void Listener::onConnect() {
        struct sockaddr_in peeraddr{};
        socklen_t addrlen = sizeof peeraddr;
        peerfd = accept(activefd, (struct sockaddr *) &peeraddr, &addrlen);
        if (peerfd == -1) {
            LOGERROR("accept error: %s", ERRNOSTR);
        } else {
            LOGINFO("new peer from " SOCKADDR_FMT, SOCKADDR_OF(peeraddr));
        }
    }

    void Listener::onDisconnect() {
        LOGINFO("peer leave");
        close(peerfd);
        peerfd = -1;
        onIdle();
        r.reset();
        heartbeat.reset();
        metrics.ops = 0;
    }

    void Listener::onUnpackError() {
        LOGERROR("unpack error");
        io::MsgResponse res("invalid message", {});
        io::MsgRaw raw(io::MSGRESPONSE);
        raw.payload = res.pack();
        w.pub(raw.move());
        msghub.notify(kNotifySmall);
    }

    void Listener::runOnLoop(Closure closure) {
        if (closure) {
            mtx.lock();
            closures.emplace_back(std::move(closure));
            mtx.unlock();
            msghub.notify(kNotifyMedium);
        }
    }

    void Listener::run() {
        worker.start();
#define POLLFDLEN 3
#define POLLFDSET(x, _1, _2) (x).fd=(_1),(x).events=(_2)
        struct pollfd pfds[POLLFDLEN]{};
        int rc;
        eventfd_t ev;
        POLLFDSET(pfds[0], msghub.fd(), POLLIN);
        POLLFDSET(pfds[1], activefd, POLLIN);
        POLLFDSET(pfds[2], peerfd, POLLIN);
        for (;;) {
            POLL(rc, poll, pfds, POLLFDLEN - idle(), HEARTBEAT_DURATION);
            if (rc == 0) {
                if (idle()) {
                    metrics.idle += HEARTBEAT_DURATION;
                } else {
                    metrics.work += HEARTBEAT_DURATION;
                    if (!heartbeat.ping(peerfd, &r)) {
                        onDisconnect();
                    }
                }
            } else {
                if (pfds[0].revents & POLLIN) {
                    ev = msghub.retrieve();
                    if (ev >= kNotifyLarge) {
                        break;
                    } else if (ev >= kNotifyMedium) {
                        std::vector<Closure> t;
                        mtx.lock();
                        t.swap(closures);
                        mtx.unlock();
                        for (auto &&fn:t) {
                            fn();
                        }
                    } else if (ev >= kNotifySmall) {
                        w.write();
                    }
                }
                if (state.load(std::memory_order_relaxed) == State::QUIT) {
                    break;
                }
                Defer switchstate;
                if (pfds[1].revents & POLLIN) {
                    onConnect();
                    if (peerfd == -1) {
                        continue;
                    }
                    metrics.times++;
                    pfds[2].fd = peerfd;
                    switchstate = Defer([this] { onWork(); });
                }
                if (work() && (pfds[2].revents & POLLIN)) {
                    r.read();
                }
            }
        }
    }

    void *Listener::Work(void *arg) {
#define MAXTESTTIMEOUT 10
        static int timeout = -1;
        static const char *a[MAXTESTTIMEOUT] = {
                "photo",
                "view",
                "cave",
                "run",
                "maximum",
                "hair",
                "chef",
                "horror",
                "hand",
                "frozen",
        };
        auto listener = reinterpret_cast<Listener *>(arg);
        // STEP1,retrieve input
        const auto &res = listener->r.msgreq;
        LOGINFO("input %d,%d", res.op, res.arg);
        // STEP2,do sth.
        if (++timeout == MAXTESTTIMEOUT) {
            timeout = 0;
        }
        util::Sleep(timeout * 1000);
        // STEP3,prepare output
        io::MsgResponse req(a[timeout], {9, 9, 6, 5});
        io::MsgRaw raw(io::MSGRESPONSE);
        raw.payload = req.pack();
        listener->w.pub(raw.move());
        listener->msghub.notify(kNotifySmall);
        return arg;
    }
}