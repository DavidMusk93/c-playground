#include "timer.h"

#include <sys/timerfd.h>
#include <unistd.h>

namespace sun {
    Timer::Config::operator itimerspec() const {
#define THOUSAND 1000
#define SETTIMESPEC(__ts, __x, __y, __b) \
if(__y>__b){\
    __x+=__y/__b;\
    __y%=__b;\
}\
__ts.tv_sec=__x;\
if(__y){\
    __ts.tv_nsec=__y*__b*__b;\
}
        struct itimerspec its{};
        u32 a1, a2, b1, b2;
        a1 = interval_second_, a2 = interval_millisecond_;
        b1 = value_second_, b2 = value_millisecond_;
        SETTIMESPEC(its.it_interval, a1, a2, THOUSAND);
        SETTIMESPEC(its.it_value, b1, b2, THOUSAND);
        return its;
#undef SETTIMESPEC
#undef THOUSAND
    }

    Timer::Timer(const Config &config, u32 flags) {
        ERRRET((fd_ = timerfd_create(CLOCK_REALTIME, flags)) == -1, , , 1, "timerfd_create");
        auto its = (struct itimerspec) config;
        timerfd_settime(fd_, 0, &its, nullptr);
        cleanup_ = Defer([this] { util::Close(fd_); });
    }

    void Timer::OnTimeout(int fd) {
        unsigned long expiration{};
        read(fd, &expiration, sizeof(expiration));
        FUNCLOG("%lu", expiration);
    }
}