#include "macro.h"

#include <unistd.h>

void on_quit(){
    LOG("ops...");
}

MAIN(){
    atexit(&on_quit);
    int i=100;
    sun::Defer defer([&i]{
        LOG("Hi,there! %d",i);
    });
    sleep(10);
    return 0;
}