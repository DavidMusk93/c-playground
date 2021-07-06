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
            Config() : Config(0, 0, 0, 0) {}

            explicit Config(u32 interval_second, u32 interval_millisecond = 0, u32 value_second = 5,
                            u32 value_millisecond = 0)
                    : INITMEMBER(interval_second), INITMEMBER(interval_millisecond),
                      INITMEMBER(value_second), INITMEMBER(value_millisecond) {}

            explicit operator struct itimerspec() const;

            bool notnull() const;

        private:
            u32 interval_second_; // timer interval
            u32 interval_millisecond_;
            u32 value_second_; // initial expiration
            u32 value_millisecond_;

        public:
            static constexpr const long kThousand = 1000;
            static constexpr const long kMillion = kThousand * kThousand;
        };

        explicit Timer(const Config &cfg, u32 flags = O_CLOEXEC | O_NONBLOCK);

        ~Timer() override = default;

        int reset(const Config &cfg);

//        double leftMillis();

        static void OnTimeout(int fd);

        static constexpr const u32 kLeastDuration = 10; /*ms*/
    };
}

#endif //MONITOR_TIMER_H
