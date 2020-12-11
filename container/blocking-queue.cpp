//
// Created by Steve on 10/27/2020.
//

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>

#include "macro.h"

using namespace std;

template<typename T>
class BQ{
#define LOCK unique_lock<mutex> lock(mtx_)
#define WAIT cv_.wait
#define NOTIFY cv_.notify_all()
public:
    T get(){
        LOCK;
        WAIT(lock,[this]{return !q_.empty();}); /*same as while(q_.empty()){cv_wait(lock);}*/
        T t=q_.front();
        q_.pop();
        if(q_.empty()){ /*avoid memory leak by container*/
            queue<T> eq;
            eq.swap(q_);
        }
        return t;
    }
    void put(T&t){
        LOCK;
        q_.push(move(t));
        NOTIFY;
    }
private:
    queue<T> q_;
    mutex mtx_;
    condition_variable cv_;
#undef NOTIFY
#undef WAIT
#undef LOCK
};

MAIN(){
    BQ<int> bq;
    auto consume=[&bq](int id){
        LOG("#%d,%d",id,bq.get());
    };
    const int N=10;
    auto produce=[&bq,&N](){
        for(int i=0;i<N;++i){
            bq.put(i);
        }
    };
    vector<thread> threads;
    for(int i=0;i<N;++i){
        threads.emplace_back(consume,i);
    }
    threads.emplace_back(produce);
    for(auto&t:threads){
        t.join();
    }
}