//
// Created by Steve on 8/25/2020.
//

#include "tool.h"

namespace sun::time{
    timeval now(){
        timeval tv{};
        gettimeofday(&tv,nullptr);
        return tv;
    }

    double seconds(const timeval&tv){
        return tv.tv_sec+tv.tv_usec/1000000.;
    }

    const char*format(const timeval&tv){
        static __thread char buf[64];
        sprintf(buf,"%ld.%06ld",tv.tv_sec,tv.tv_usec);
        return buf;
    }
}

Signal & Signal::registerSignal(int signum) {
    sigaddset(&mask_,signum);
    return *this;
}

int Signal::fd() {
    if(fd_==-1){
        fd_=signalfd(-1,&mask_,SFD_NONBLOCK);
    }
    sigprocmask(SIG_BLOCK,&mask_,nullptr);
    return fd_;
}

Counter::Counter() {
    count_=0;
    ts_=SECONDS;
}

void Counter::tick() {
    if(++count_==MAX){
        auto ts=SECONDS;
        printf("@FREQUENCY %f\n",count_/(ts-ts_));
        Reset(ts);
    }
}