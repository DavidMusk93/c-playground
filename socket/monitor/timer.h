#ifndef MONITOR_TIMER_H
#define MONITOR_TIMER_H

#include <time.h>
#include <fcntl.h>

#include "endpoint.h"

namespace sun {
    using u32 = unsigned;

    class Timer : public EndPoint {
    public:
        struct Config {
            Config(u32 interval_second, u32 interval_millisecond = 0, u32 value_second = 5, u32 value_millisecond = 0)
                    : INITMEMBER(interval_second),
                      INITMEMBER(interval_millisecond),
                      INITMEMBER(value_second),
                      INITMEMBER(value_millisecond) {}

            explicit operator struct itimerspec() const;

        private:
            u32 interval_second_; // timer interval
            u32 interval_millisecond_;
            u32 value_second_; // initial expiration
            u32 value_millisecond_;
        };

        Timer(const Config &config, u32 flags = O_CLOEXEC | O_NONBLOCK);

        ~Timer() override = default;

        static void OnTimeout(int fd);
    };
}

#endif //MONITOR_TIMER_H
