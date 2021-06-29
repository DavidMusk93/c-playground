#include "macro.h"

#include <pthread.h>
#include <signal.h>

//long a;
int q=0;

void quit(int sig){
    q=sig;
}

#define V1 0x1122334455667788
#define V2 0x5566778811223344

void*setval1(void*arg){
    while(!q)
        *(long*)arg=V1;
    return 0;
}

void*setval2(void*arg){
    while(!q)
        *(long*)arg=V2;
    return 0;
}

MAIN(){
    long a=V1;
    pthread_t t1,t2;
    pthread_create(&t1,0,&setval1,&a);
    pthread_create(&t2,0,&setval2,&a);
    signal(SIGINT,&quit);
    while(!q){
        long b=a;
        if(!(b==V1||b==V2)){
            LOG("%#lx",b);
            q=1;
        }
    }
    pthread_join(t1,0);
    pthread_join(t2,0);
    return 0;
}