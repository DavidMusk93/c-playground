//
// Created by Steve on 12/2/2020.
//

#include "macro.h"

namespace trivial{
    class Lock{
    public:
        virtual void lock(){}
        virtual void unlock(){}
    };

    class LockGuard{
    public:
        LockGuard(Lock&lock):lock_(lock){
            lock_.lock();
        }
        ~LockGuard(){
            lock_.unlock();
        }

    private:
        Lock&lock_;
    };
}

class SQLMXLoggingArea{
public:
    bool lockMutex(){
        LOG("lock");
        return true;
    }
    void unlockMutex(){
        LOG("unlock");
    }
    void criticalWork();
};

class Lock:public trivial::Lock{
public:
    Lock(SQLMXLoggingArea&ref):ref_(ref),state_(false){}
    void lock(){
        state_=ref_.lockMutex();
    }
    void unlock(){
        if(state_){
            ref_.unlockMutex();
        }
    }
private:
    SQLMXLoggingArea&ref_;
    bool state_;
};

void SQLMXLoggingArea::criticalWork() {
    Lock lock(*this);
    trivial::LockGuard lockGuard(lock);
    LOG("sth. critical");
}

MAIN(){
    SQLMXLoggingArea obj;
    obj.criticalWork();
}