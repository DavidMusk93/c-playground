#include "listener.h"
#include "util.h"
#include "sock.h"

namespace sun {
#define NOW() sun::util::Milliseconds()

    bool Heartbeat::ping(int fd, Listener *ctx) {
        auto now = NOW();
        if (now - ctx->r.lastest_readts < HEARTBEAT_DURATION) { /*no need to ping*/
            return true;
        }
        if (!retry) {
            return false;
        }
        if (lastest_pingts > 0 && (
                rtt < 0 /*first ping, without pong*/||
                now - lastest_pongts > HEARTBEAT_DURATION + 10 * maxrtt /*pong timeout*/)) {
            --retry;
            LOGINFO("no pong form peer(%.3f,%.3f,%.3f), remaining retry: %d", rtt, minrtt, maxrtt, retry);
        }
        LOGINFO("send ping");
        io::MsgHeartbeat msg(now);
        io::MsgRaw raw(io::MSGPING);
        raw.payload = msg.pack();
//        if (raw.write(fd)) { /*write should run in io-thread*/
//            lastest_pingts = now;
//            return true;
//        }
//        return false;
        lastest_pingts = now;
        ctx->w.pub(raw.move(), ctx);
        return true;
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
        if (maxrtt < d && d < HEARTBEAT_DURATION / 20.) {
            maxrtt = d;
        }
        retry = HEARTBEAT_MAXRETRY;
    }

    void Reader::read() {
        io::MsgRaw raw;
        if (raw.read(listener->peerfd)) {
            auto now = NOW();
            io::Unpacker r(raw.payload);
            switch (raw.type) {
                case io::MSGREQUEST: {
                    if (msgreq.unpack(&r)) {
                        lastest_readts = now;
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
                        LOGINFO("receive pong rrt:%.3fms", listener->heartbeat.rtt);
                    } else {
                        listener->onUnpackError();
                    }
                    break;
                }
                default:
                    LOGERROR("unknown message type: %d", raw.type);
            }
            /*it is better to read as more as possible?*/
        } else {
            listener->onDisconnect();
        }
    }

    void Writer::pub(io::MsgRaw &&msg, Listener *ctx) {
        mtx.lock();
        msgs.emplace_back(msg.move());
        mtx.unlock();
        ctx->msghub.notify(Listener::kNotifySmall);
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
                                     r(this), w(this), metrics() {
        io::TcpipServer server(port);
        if (server.valid()) {
            activefd = server.transferOwnership();
            setstate(State::INITIALIZED);
        }
    }

    Listener::~Listener() {
        if (initialized()) {
            msghub.notify(kNotifyLarge);
            timer.stop();
            worker.stop();
            if (activefd != -1) {
                close(activefd);
            }
        }
    }

    bool Listener::idle() const {
        return getstate() == State::IDLE;
    }

    bool Listener::work() const {
        return getstate() == State::WORK;
    }

    void Listener::onWork() {
        setstate(State::WORK);
    }

    void Listener::onIdle() {
        setstate(State::IDLE);
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
        w.pub(raw.move(), this);
    }

    void Listener::runOnLoop(Closure closure) {
        if (closure) {
            mtx.lock();
            closures.emplace_back(std::move(closure));
            mtx.unlock();
            msghub.notify(kNotifyMedium);
        }
    }

    void Listener::runAfter(fn_job_t job, void *arg, unsigned int ms, int type) {
        if (!job || (type != TIMERTASK_ONCE && type != TIMERTASK_REPEATED)) {
            return;
        }
        timer.post({ms, type, job, arg});
    }

    void Listener::run() {
        worker.start();
        timer.start();
#define POLLFDLEN 4
#define POLLFDSET(x, _1, _2) (x).fd=(_1),(x).events=(_2)
        struct pollfd pfds[POLLFDLEN]{};
        auto &activepfd = pfds[2];
        auto &positivepfd = pfds[3];
        int rc;
        eventfd_t ev;
        POLLFDSET(pfds[0], msghub.fd(), POLLIN);
        POLLFDSET(pfds[1], timer.fd(), POLLIN);
        POLLFDSET(activepfd, activefd, POLLIN);
        POLLFDSET(positivepfd, peerfd, POLLIN);
        LOGINFO("timer state: %d", (int) timer.getstate());
        timer.post({1000, TIMERTASK_REPEATED, tWork, nullptr}); /*timer test 1*/
        timer.post({2000, TIMERTASK_REPEATED, tWork2, nullptr}); /*timer test 2*/
        timer.post({HEARTBEAT_DURATION, TIMERTASK_REPEATED, Ping, this}); /*heartbeat*/
        for (;;) {
            POLL(rc, poll, pfds, POLLFDLEN - idle(), /*HEARTBEAT_DURATION*/-1);
#if 0
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
#endif
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
            if (getstate() == State::TERMINATED) {
                break;
            }
            if (timer.getstate() == State::WORK && (pfds[1].revents & POLLIN)) {
                Timer::OnTimeout(pfds[1].fd); /*notify*/
                timer.post(tTask::Nil); /*action*/
            }
            Defer switchstate;
            if (activepfd.revents & POLLIN) {
                onConnect();
                if (peerfd == -1) {
                    continue;
                }
                metrics.times++;
                positivepfd.fd = peerfd;
                switchstate = Defer([this] { onWork(); });
            }
            if (work() && (positivepfd.revents & POLLIN)) {
                r.read();
            }
#if 0
            }
#endif
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
        auto self = reinterpret_cast<Listener *>(arg);
        // STEP1,retrieve input
        const auto &res = self->r.msgreq;
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
        self->w.pub(raw.move(), self);
        return arg;
    }

    void *Listener::tWork(void *) {
        FUNCLOG("run per second");
        return nullptr;
    }

    void *Listener::tWork2(void *) {
        FUNCLOG("run every 2 seconds");
        return nullptr;
    }

    void *Listener::Ping(void *arg) {
        LOGINFO("@HEARTBEAT ping task");
        auto self = reinterpret_cast<Listener *>(arg);
        if (self->idle()) {
            self->metrics.idle += HEARTBEAT_DURATION;
        } else {
            self->metrics.work += HEARTBEAT_DURATION;
            if (!self->heartbeat.ping(self->peerfd, self)) {
                self->onDisconnect();
            }
        }
        return arg;
    }
}