//
// Created by Steve on 8/1/2020.
//

#include <thread>
#include <vector>

#include "macro.h"

#define TLOG(fmt,...) LOG("%ld" fmt,(long)pthread_self(),##__VA_ARGS__)

void dtor(void*v){
    TLOG("@%s %d",__func__,*(int*)v);
}

MAIN(){
    pthread_key_t pk;
    pthread_key_create(&pk,&dtor);
    auto a=[&pk](intptr_t arg){
        if(arg){
            pthread_setspecific(pk, (void*)(arg));
        }
        TLOG("quit");
    };
    auto b=a;
    auto c=a;
    std::vector<std::thread> threads;
    std::vector<int> data{1,2,3};
    auto it=data.begin();
    for(auto&f:{a,b,c}){
        threads.emplace_back(std::thread(f,(intptr_t)&*it++));
    }
    threads.emplace_back(std::thread(a,(intptr_t)0));
    for(auto&t:threads){
        t.join();
    }
    return 0;
}