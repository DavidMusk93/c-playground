//
// Created by Steve on 9/17/2020.
//

#include <ucontext.h>
#include <unistd.h>

#include "macro.h"

typedef void (fn_t)(void);
typedef void (*fn_ptr_t)(void);

#define FN_TAG(tag) LOG("@%s " #tag,__func__)

void foo(void*p1,void*p2){
    FN_TAG(START);
//    ucontext_t*uc1,*uc2;
//    uc1=p1,uc2=p2;
    LOG("yield");
    swapcontext(p2,p1);
    FN_TAG(END);
    setcontext(p1);
}

MAIN(){
//    ucontext_t uctx;
//    getcontext(&uctx);
//    LOG("again");
//    sleep(1);
//    setcontext(&uctx);
//    LOG("%p,%p",(fn_t*)&foo,(fn_ptr_t)foo);
//    LOG("%p,%p",(fn_t*)foo,(fn_ptr_t)&foo);
    ucontext_t uc[2];
    char st[8192];
    getcontext(&uc[1]);
    uc[1].uc_link=&uc[0];
    uc[1].uc_stack.ss_sp=st;
    uc[1].uc_stack.ss_size=sizeof st;
    makecontext(&uc[1],(fn_ptr_t)&foo,2,&uc[0],&uc[1]);
    FN_TAG(START);
    swapcontext(&uc[0],&uc[1]);
    LOG("resume");
    swapcontext(&uc[0],&uc[1]);
    FN_TAG(END);
    return 0;
}