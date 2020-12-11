//
// Created by Steve on 11/16/2020.
//

#include <stdlib.h>
//#include <poll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <assert.h>

#include <thread>

#include "macro.h"

MAIN(){
    const char*key="TEST";
    int v=12;
    int notifier=eventfd(0,0);
    sun::Defer defer([&notifier]{close(notifier);});
    auto task=std::thread([key,&notifier](int value){
        char buf[64];
        unsigned long u=1;
        sprintf(buf,"%s=%d",key,value);
        putenv(buf);
        write(notifier,&u,sizeof(u));
    },v);
    unsigned long u{};
    read(notifier,&u,sizeof(u));
    assert(u==1);
    LOG("@ENV %s=%s",key,getenv(key));
    task.join();
    return 0;
}