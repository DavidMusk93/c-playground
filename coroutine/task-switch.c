//
// Created by Steve on 9/17/2020.
//

#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/time.h>

#include "macro.h"

static volatile int expired;
static ucontext_t uc[3];
static int switches;

static void fn(int n){
    int m=0;
    while(1){
        if(++m%100==0){
            putchar('.');
            fflush(stdout);
        }
        if(expired){
            if(++switches==20){return;}
            LOG("\nswitching from %d to %d",n,3-n);
            expired=0;
            swapcontext(&uc[n],&uc[3-n]);
        }
    }
}

typedef void (*task_t)(void);

void handler(int sig){
    expired=1;
}

MAIN(){
    struct sigaction sa;
    struct itimerval it;
    char st1[8192],st2[8192];
    sa.sa_flags=SA_RESTART;
    sigfillset(&sa.sa_mask);
    sa.sa_handler=&handler;
    it.it_interval.tv_sec=1;
    it.it_interval.tv_usec=0;
    it.it_value=it.it_interval;
    if(sigaction(SIGPROF,&sa,NULL)<0||setitimer(ITIMER_PROF,&it,NULL)<0){
        abort();
    }
#define SETUCONTEXT(n) \
getcontext(&uc[n]);\
uc[n].uc_link=&uc[0];\
uc[n].uc_stack.ss_sp=st##n;\
uc[n].uc_stack.ss_size=sizeof st##n;\
makecontext(&uc[n],(task_t)&fn,1,n)
    SETUCONTEXT(1);
    SETUCONTEXT(2);
    swapcontext(&uc[0],&uc[1]);
    putchar('\n');
    return 0;
}