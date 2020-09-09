//
// Created by Steve on 9/4/2020.
// USAGE:
//   g++ timer.cpp -I.. -lboost_system -lpthread
//   ./a.out | ../io/timestamp
//

#include <boost/asio.hpp>

using namespace boost;

class Timer:asio::noncopyable,public std::enable_shared_from_this<Timer>{
public:
    using callback=std::function<void(bool)>;

public:
    Timer(asio::io_service&is,size_t timeout):timer_(is),timeout_(timeout){}

    void start(){
        if(timeout_==0){
            return;
        }
        timer_.expires_from_now(std::chrono::milliseconds(timeout_));
        auto self=this->shared_from_this();
        timer_.async_wait([this,self](system::error_code ec){
            Callback(!ec);
        });
    }

    void register_callback(callback cb){
        cb_.swap(cb);
    }

    bool cancel(){
        system::error_code ec;
        timer_.cancel(ec);
        return !ec;
    }

private:
    void Callback(bool expire){
        if(cb_){cb_(expire);}
    }

private:
    asio::steady_timer timer_;
    callback cb_;
    size_t timeout_;
};

#include "macro.h"

MAIN(){
    REVOKE_OUTPUT_BUFFER();
    asio::io_context ioc;
    auto timer=std::make_shared<Timer>(ioc,1000);
    std::weak_ptr<Timer> wp(timer);
    timer->register_callback([wp](bool expire){
        if(expire){
            LOG("timeout & restart");
            wp.lock()->start();
        }else{
            LOG("done");
        }
    });
    timer->start();
    ioc.run();
}