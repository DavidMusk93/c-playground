#include "util.h"

#include <time.h>
#include <signal.h>

namespace sun {
    namespace details {
        const char *DefaultTimeFormatter(char buf[32], int seconds, int milliseconds) {
            static thread_local char __internal_buf[32];
            char *p = buf ?: __internal_buf;
            sprintf(p, "%d.%03d", seconds, milliseconds);
            return p;
        }
    }
    namespace utility {
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
    }
}