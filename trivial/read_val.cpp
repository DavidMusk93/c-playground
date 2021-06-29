#include "macro.h"

#include <thread>
#include <signal.h>

struct A{
    void*a;
    int b;
    char c;
    long d;
};

int q=0;
void oninterrupt(int sig){
    q=1;
}

MAIN(){
    long x,y;
    A a{.a=&x};
    auto t1=std::thread([&a](void*p){
        while(!q){
            a.a=p;
        }
    },&x);
    auto t2=std::thread([&a](void*p){
        while(!q){
            a.a=p;
        }
    },&y);
    signal(SIGINT,&oninterrupt);
    while(!q){
        auto p=a.a;
        if(p!=&x&&p!=&y){
            LOG("%p",p);
            q=1;
        }
    }
    t1.join();
    t2.join();
    return 0;
}