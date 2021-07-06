#include "timer.h"

#include <sys/timerfd.h>
#include <unistd.h>

namespace sun {
    Timer::Config::operator struct itimerspec() const {
#define THOUSAND 1000
#define SETTIMESPEC(__ts, __x, __y) \
if(__y>=kThousand){\
    __x+=__y/kThousand;\
    __y%=kThousand;\
}\
__ts.tv_sec=__x;\
if(__y){\
    __ts.tv_nsec=__y*kMillion;\
}
        struct itimerspec its{};
        u32 a1, a2, b1, b2;
        a1 = interval_second_, a2 = interval_millisecond_;
        b1 = value_second_, b2 = value_millisecond_;
        SETTIMESPEC(its.it_interval, a1, a2);
        SETTIMESPEC(its.it_value, b1, b2);
        return its;
#undef SETTIMESPEC
#undef THOUSAND
    }

    bool Timer::Config::notnull() const {
        return interval_second_
               || interval_millisecond_ >= Timer::kLeastDuration
               || value_second_
               || value_millisecond_ >= Timer::kLeastDuration;
    }

    Timer::Timer(const Config &cfg, u32 flags) {
        ERRRET((fd_ = timerfd_create(CLOCK_REALTIME, flags)) == -1, , , 1, "timerfd_create");
        if (cfg.notnull()) {
            auto its = (struct itimerspec) cfg;
            timerfd_settime(fd_, 0, &its, nullptr);
        }
        cleanup_ = Defer([this] { util::Close(fd_); });
        initialize();
    }

    int Timer::reset(const Config &cfg) {
        auto its = (struct itimerspec) cfg;
        return timerfd_settime(fd_, 0, &its, nullptr);
    }

    void Timer::OnTimeout(int fd) {
        unsigned long expiration{};
        read(fd, &expiration, sizeof(expiration));
        FUNCLOG("%lu", expiration);
    }
}