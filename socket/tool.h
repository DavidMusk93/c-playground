//
// Created by Steve on 8/25/2020.
//

#ifndef C4FUN_TOOL_H
#define C4FUN_TOOL_H

#include <unistd.h>
#include <sys/signal.h>
#include <sys/signalfd.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>


#define NOW() ({\
    timeval tv{};\
    gettimeofday(&tv,nullptr);/*VDSO(Virtual Dynamic Shared Object)*/\
    tv;\
})
#define SECONDS sun::time::seconds(NOW())

#define TIMETHIS(expr) \
do{\
    auto t1=SECONDS;\
    expr;\
    auto t2=SECONDS;\
    printf(#expr "cost #%f (s)\n",t2-t1);\
}while(0)

namespace sun::time{
    extern timeval now();
    extern double seconds(const timeval&tv);
    extern const char*format(const timeval&tv);
}

class Signal{
public:
    Signal(){sigemptyset(&mask_);}
    Signal&registerSignal(int signum);
    int fd();

public:
    static signalfd_siginfo Read(int fd){
        signalfd_siginfo ss{};
        read(fd,&ss,sizeof(ss));
        return ss;
    }

private:
    sigset_t mask_{};
    int fd_{-1};
};

class Counter{
public:
    Counter();
    void tick();

private:
    void Reset(double ts){
        count_=0;
        ts_=ts;
    }

private:
    size_t count_;
    double ts_;

    static constexpr const size_t MAX=30;
};

#endif //C4FUN_TOOL_H
