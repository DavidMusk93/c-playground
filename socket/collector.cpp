//
// Created by Steve on 8/24/2020.
//

#include "network.h"

#include <deque>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <type_traits>

namespace sun{
    class noncopyable{
    public:
        noncopyable(const noncopyable&)=delete;
        noncopyable&operator=(const noncopyable&)=delete;

    protected:
        noncopyable()=default;
        ~noncopyable()=default;
    };

    class Spinlock{
    public:
        Spinlock(std::atomic_flag&flag):flag_(flag){
            while(flag_.test_and_set(std::memory_order_acquire));
        }
        ~Spinlock(){
            flag_.clear(std::memory_order_release);
        }

    private:
        std::atomic_flag&flag_;
    };

    template<typename T>
    class Queue final:public noncopyable{
#define LOCK() std::unique_lock<std::mutex> lock(mutex_)
    public:
//        Queue()=default;
        ~Queue(){finish();}
        void offer(T&&t){
            LOCK();
            data_.push_back(/*std::forward<T>(t)*/std::move(t));
            cv_.notify_one();
        }
        bool poll(T&t){
            LOCK();
            cv_.wait(lock,[this](){return finish_||!data_.empty();});
            if(finish_){
                return false;
            }
            t.swap(data_.front());
            data_.pop_front();
            return true;
        }
        void finish(){
            LOCK();
            finish_=true;
            cv_.notify_all();
        }
    private:
        std::deque<T> data_;
        std::mutex mutex_;
        std::condition_variable cv_;
        bool finish_{false};
    };

    template<size_t N,size_t BLOCK>
    class Pool final:public noncopyable{
    public:
//        using header_t=size_t ;
        using block_t=void*;
        using release_t=std::function<void(block_t)>;
        using value_type=std::unique_ptr<void*,release_t>;

        Pool():/*size_(0),capacity_(N),*/block_(BLOCK){
            static_assert(N<=1024&&BLOCK<=1024*1024);
            const size_t MASK=sizeof(void*)-1;
//            if(block_&MASK){
//                block_=(block_&(~MASK))+sizeof(header_t)*2;
//            }else{
//                block_+=sizeof(header_t);
//            }
            if(block_&MASK){
                block_=(block_&(~MASK))+sizeof(void*);
            }
            data_=malloc(N*block_);
            stack_.resize(N);
            for(size_t i=0;i<N;++i){
                stack_[i]=reinterpret_cast<void*>((char*)data_+i*block_);
            }
        }
        ~Pool(){
            free(data_);
        }

        value_type acquire(){
            Spinlock sl{flag_};
            if(!stack_.empty()){
                void*v=stack_.back();
                stack_.pop_back();
                return value_type{new(v)block_t,[this](void*v){Spinlock sl{flag_};stack_.push_back(v);}};
            }else{
                void*v=malloc(block_);
                return value_type{new(v)block_t,[](void*v){free(v);}};
            }
        }

    private:
        void*data_;
//        size_t size_;
//        size_t capacity_;
        size_t block_;
        std::vector<void*> stack_;
        std::atomic_flag flag_=ATOMIC_FLAG_INIT;
    };
}

class Collector{
public:
#define TAG "(COLLECTOR)"
//    struct Payload{
//        char buf[64]{};
//        int len{};
//    };
    Collector(const std::string&server_ip,short server_port):fd_(-1),epoll_handler_(-1){
        int fd;
        ERROR_RETURN((fd=socket(AF_INET,SOCK_STREAM,0))==-1,,,1);
        MAKE_SOCKADDR_IN(server,inet_addr(server_ip.c_str()),htons(server_port));
        Cleaner cleaner{[&fd]{close(fd);}};
        struct timeval tv{2,0};
        SETSOCKOPT(fd,SOL_SOCKET,SO_SNDTIMEO,tv,);
        LOG("(COLLECTOR)connect to " SOCKADDR_FMT,SOCKADDR_OF(server));
        ERROR_RETURN(connect(fd,SOCKADDR_EX(server))==-1,,,1);
        cleaner.cancel();
        fd_=fd;
        ERROR_RETURN((epoll_handler_=epoll_create1(0))==-1,,,1);
    }
    ~Collector(){
        quit();
        FdHelper::Close(fd_);
        FdHelper::Close(epoll_handler_);
    }
    bool registerSource(int fd,decltype/*build-in?*/(std::declval<Connection::Callback>().on_recv) on_recv){
        if(epoll_handler_==-1){
//            LOG("(COLLECTOR)epoll instance has not created");
            close(fd);
            return false;
        }
        auto handler=std::make_shared<Connection>(fd);
        ERROR_RETURN(handler->epollRegister(epoll_handler_,EPOLLIN|EPOLLET|EPOLLRDHUP)==-1,false,,0);
        handler->registerCallback({std::move(on_recv),nullptr,[handler,this](int&){handlers_.erase(handler);}},this);
        handlers_.insert(handler);
        return true;
    }
    void addPayload(std::string payload){
        payloads_.offer(std::move(payload));
    }

    void loop(){
        if(Ready()){
//            producer_=std::thread(&Collector::Produce,this);
            consumer_=std::thread(&Collector::Consume,this);
            Produce();
        }
    }

    void quit(){
#define JOIN(x) \
do{\
    if(x.joinable()){x.join();}\
}while(0)
        payloads_.finish();
        terminator_.trigger();
//        JOIN(producer_);
        JOIN(consumer_);
#undef JOIN
    }

public:
    static void HandleGroupMsg(int fd, void*user_data);
    static void HandlerInterrupt(int fd, void*user_data);

protected:
    void Produce(){
#define MAX_EVENTS 10
        int nfds;
        struct epoll_event events[MAX_EVENTS];
        Connection tmp{terminator_.observeFd()};
        ERROR_RETURN(tmp.epollRegister(epoll_handler_,EPOLLIN)==-1,,,0);
        tmp.cancel();
        tmp=Connection{fd_};
        ERROR_RETURN(tmp.epollRegister(epoll_handler_,EPOLLRDHUP)==-1,,,0);
        tmp.cancel();
        for(;;){
            ERROR_RETURN((nfds=epoll_wait(epoll_handler_,events,MAX_EVENTS,-1))==-1,,,1);
            for(int i=0;i<nfds;++i){
                const auto&fd=events[i].data.fd;
                if(fd==fd_&&events[i].events&EPOLLRDHUP){
                    LOG(TAG "remote server has disconnected");
                    goto end;
                }
                if(fd==terminator_.observeFd()){
                    terminator_.cleanup();
                    goto end;
                }
                for(auto&handler:handlers_){
                    if(handler->fd()==fd){
                        handler->eventTrigger(epoll_handler_,events[i].events);
                    }
                }
            }
        }
end:
        return;
    }
    void Consume(){
        for(;;){
            std::string payload;
            if(!payloads_.poll(payload)){
                break;
            }
            send(fd_,payload.data(),payload.size(),0/*MSG_MORE*/);
        }
    }

private:
    bool Ready() const{
        return fd_!=-1&&epoll_handler_!=-1&&!handlers_.empty();
    }

private:
    int fd_;
    int epoll_handler_;
    sun::Queue<std::string> payloads_;
    std::unordered_set<Connection::Handler> handlers_;
    Terminator terminator_;
    std::thread /*producer_,*/consumer_;
};

void Collector::HandleGroupMsg(int fd, void *user_data){
    auto collector=reinterpret_cast<Collector*>(user_data);
    char buf[64];
    int nr;
    struct sockaddr_in from{};
    socklen_t len{sizeof(from)};
    nr=recvfrom(fd,buf,sizeof(buf),0,(struct sockaddr*)&from,&len);
    LOG(TAG "receive '%.*s' from " SOCKADDR_FMT,nr,buf,SOCKADDR_OF(from));
    collector->addPayload(std::string(buf,nr));
}

void Collector::HandlerInterrupt(int fd, void *user_data) {
    auto collector=reinterpret_cast<Collector*>(user_data);
    auto ss=Signal::Read(fd);
    LOG(TAG "receive signal #%d",ss.ssi_signo);
    collector->quit();
}
#undef TAG

template<typename T>
std::vector<T> split(const char*s,char separator,const std::function<T(const std::string&)>& op){
    std::vector<T> res;
    size_t i,j;
    for(i=0,j=0;s[j];++j){
        if(s[j]==separator){
            if(j>i){
                res.push_back(op(std::string{&s[i],j-i}));
            }
            i=j+1;
        }
    }
    if(j>i){
        res.push_back(op(std::string{&s[i],j-i}));
    }
    return res;
}

int main(int argc,char*argv[]){
//    sun::Pool<4,sizeof(int)> pool;
//    std::vector<decltype(pool)::value_type> data;
//    data.reserve(10);
//    for(int i=0;i<6;++i){
//        data.emplace_back(pool.acquire());
//    }
//    for(auto&uptr:data){
//        LOG("%p",uptr.get());
//        *(int*)uptr.get()=666;
//    }
//    auto uptr=pool.acquire();
//    LOG("%p",uptr.get());
#define PORT_BASE 10000
#define GROUP_IP "228.67.43.91"
#define SERVER_IP "129.28.174.124"
#define SERVER_PORT 10001
    if(argc<2){
        return 1;
    }
//    const char*ports=argv[1];
//    LOG("%s",ports);
//    for(auto&i:split<int>(ports,',',[](const std::string&s)->int{return std::stoi(s);})){
//        LOG("%d",i);
//    }
    auto ports=split<short>(argv[1],',',[](const std::string&s)->short{return (std::stoi(s)&/*lower precedence*/0xff)+PORT_BASE;});
    Collector collector{SERVER_IP,SERVER_PORT};
    collector.registerSource(Signal().registerSignal(SIGINT).registerSignal(SIGQUIT).fd(), &Collector::HandlerInterrupt);
    for(auto port:ports){
        collector.registerSource(Group(GROUP_IP,port).fd()/*transfer ownership*/, &Collector::HandleGroupMsg);
    }
    collector.loop();
//    pause();
//    collector.stop();
}