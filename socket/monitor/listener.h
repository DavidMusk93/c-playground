#ifndef MONITOR_LISTENER_H
#define MONITOR_LISTENER_H

#define HEARTBEAT_MAXRETRY 5
#define HEARTBEAT_DURATION 5000 /*ms*/

#include <string>

#include <sys/poll.h>

#include "runner.h"
#include "message.h"

namespace sun {
    struct Listener;

    struct Reader;

    struct Heartbeat {
        double lastest_pingts;
        double rtt;
        int retry;

        Heartbeat() : lastest_pingts(-1), rtt(0), retry(HEARTBEAT_MAXRETRY) {}

        bool ping(int fd, Reader *r);

        void reset();

        void updateRTT(double d);
    };

    struct Reader {
        io::MsgRequest msgreq;
        io::MsgHeartbeat msght;
        double lastest_readts;
        Listener *listener;

        explicit Reader(Listener *listener) : msgreq(), msght(), lastest_readts(-1), listener(listener) {}

        void read();

        void reset() {
            lastest_readts = -1;
        }
    };

    struct Writer {
        Listener *listener;
        std::vector<io::MsgRaw> msgs;
        std::mutex mtx;

        explicit Writer(Listener *listener) : listener(listener) {}

        void pub(io::MsgRaw &&msg);

        void write();
    };

    struct Metrics {
        long work;
        long idle;
        long times; /*entire lifetime*/
        unsigned long ops; /*per session*/
#define OPMASK (1UL<<63)
    };

    struct Listener {
        enum class State : char {
            UNINITIALIZED,
            WORK,
            IDLE,
            QUIT,
        };
        int activefd, peerfd;
        Notifier msghub;
        TaskRunner worker;
        Heartbeat heartbeat;
        Reader r;
        Writer w;
        std::atomic<State> state;
        Metrics metrics;
        std::mutex mtx;
        std::vector<Closure> closures;

        explicit Listener(short port);

        ~Listener();

        bool valid() const;

        bool idle() const;

        bool work() const;

        void onWork();

        void onIdle();

        void onConnect();

        void onDisconnect();

        void onUnpackError();

        void runOnLoop(Closure closure);

        void run();

//    private:
//        struct {
//            int timeout;
//        } cfg_;

        static void *Work(void *);

    private:
        static constexpr const eventfd_t kNotifySmall = 1UL;
        static constexpr const eventfd_t kNotifyMedium = 1UL << 31;
        static constexpr const eventfd_t kNotifyLarge = 1UL << 63;
    };
}

#endif //MONITOR_LISTENER_H
