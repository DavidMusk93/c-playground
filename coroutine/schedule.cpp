//
// Created by Steve on 9/17/2020.
//

#include <ucontext.h>

#include <vector>
#include <memory>

namespace sun{
    typedef void (*task_t)(void);

    enum State{
        RUNNABLE,
        RUNNING,
        SUSPEND,
    };

    struct Coroutine;
    struct Schedule{
        ucontext_t main;
        int running_id;
        std::vector<std::unique_ptr<Coroutine>> coroutines;

        Schedule():running_id(-1){}
    };

    struct Coroutine{
        static constexpr const int kStackSize=16*1024;
        ucontext_t uc;
        task_t task;
        void*arg;
        enum State state;
        char stack[kStackSize];

        static int Create(Schedule&sched,task_t task,void*arg);
        static void Yield(Schedule&sched);
        static void Resume(Schedule&sched,int id);
    };

    int Coroutine::Create(Schedule&sched,task_t task,void*arg){
        auto co=std::make_unique<Coroutine>();
        co->task=task;
        co->arg=arg;
        co->state=RUNNABLE;
        getcontext(&co->uc);
        co->uc.uc_link=&sched.main;
        co->uc.uc_stack.ss_sp=co->stack;
        co->uc.uc_stack.ss_size=Coroutine::kStackSize;
        makecontext(&co->uc,co->task,1,co->arg);
        sched.coroutines.push_back(std::move(co));
        return sched.coroutines.size();
    }

    void Coroutine::Yield(Schedule&sched){
        auto&co=sched.coroutines[sched.running_id-1];
        co->state=SUSPEND;
        sched.running_id=-1;
        swapcontext(&co->uc,&sched.main);
    }

    void Coroutine::Resume(Schedule&sched,int id){
        do{
            if(id<=0||id>sched.coroutines.size()){break;}
            auto&co=sched.coroutines[id-1];
            if(co->state==RUNNING){break;}
            co->state=RUNNING;
            sched.running_id=id;
            swapcontext(&sched.main,&co->uc);
        }while(0);
    }

    static Schedule sched;
    static int newCoroutine(task_t task,void*arg){
        return Coroutine::Create(sched,task,arg);
    }
    static void yield(){
        Coroutine::Yield(sched);
    }
    static void resume(int id){
        Coroutine::Resume(sched,id);
    }
    static bool finished(){
        for(auto&co:sched.coroutines){
            if(co->state!=RUNNING){
                return false;
            }
        }
        return true;
    }
}

#include "macro.h"

void fn1(void){
    LOG("1");
    sun::yield();
    LOG("11");
    sun::yield();
    LOG("111");
}

void fn2(void){
    LOG("2");
    sun::yield();
    LOG("22");
}

MAIN(){
    auto id1=sun::newCoroutine(&fn1,nullptr);
    auto id2=sun::newCoroutine(&fn2,nullptr);
    while(!sun::finished()){
        sun::resume(id1);
        sun::resume(id2);
    }
    return 0;
}