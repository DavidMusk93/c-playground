#include "util.h"
#include "pipe.h"

#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/poll.h>

#include <memory>

namespace sun {
    namespace details {
        const char *DefaultTimeFormatter(char buf[32], int seconds, int milliseconds) {
            static thread_local char __internal_buf[32];
            char *p = buf ?: __internal_buf;
            sprintf(p, "%d.%03d", seconds, milliseconds);
            return p;
        }
    }
    namespace util {
        double Milliseconds() {
            struct timespec ts{};
            clock_gettime(CLOCK_REALTIME, &ts);
            return ts.tv_sec * kThousand + ts.tv_nsec / kMillion;
        }

        bool ValidProcess(int pid) {
            return kill(pid, 0) == 0;
        }

        const char *Now(char buf[32], TimeFormatter timeFormatter) {
            struct timespec ts{};
            clock_gettime(CLOCK_REALTIME, &ts);
            return timeFormatter
                   ? timeFormatter(buf, ts.tv_sec, ts.tv_nsec / kMillion)
                   : details::DefaultTimeFormatter(buf, ts.tv_sec, ts.tv_nsec / kMillion);
        }

        int GetPid() {
            static thread_local int pid = ::getpid();
            return pid;
        }

        int Sleep(int ms) {
            static std::unique_ptr<Pipe> pipe;
            if (!pipe) {
                pipe.reset(new Pipe());
            }
            struct pollfd pfd{.fd=pipe->readEnd(), .events=POLLIN};
            return poll(&pfd, 1, ms);
        }

        TimeThis::TimeThis(std::string tag) : ms_(0), tag_(std::move(tag)) {
            FUNCLOG("START,%s", tag_.c_str());
            OnStart();
        }

        TimeThis::~TimeThis() {
            FUNCLOG("STOP,%s,%g", tag_.c_str(), OnStop());
        }
    }
}