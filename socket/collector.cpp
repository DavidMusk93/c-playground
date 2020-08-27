//
// Created by Steve on 8/24/2020.
//

#include "collector.h"
#include "codec.h"

#define _CALL_TAG(tag) LOG(TAG "@%s call " #tag,__func__)
#define CALL_START() _CALL_TAG(START)
#define CALL_END()   _CALL_TAG(END)

void Collector::HandleGroupMsg(int fd, void *user_data){
    auto collector=reinterpret_cast<Collector*>(user_data);
    char buf[64];
    int nr;
    struct sockaddr_in from{};
    socklen_t len{sizeof(from)};
    nr=recvfrom(fd,buf,sizeof(buf),0,(struct sockaddr*)&from,&len);
    LOG(TAG "receive '%.*s' from " SOCKADDR_FMT,nr,buf,SOCKADDR_OF(from));
    auto input=Codec::Input{};
    input.parseIp(inet_ntoa(from.sin_addr)).parseWeight(buf).parseBlock(buf);
//    collector->addPayload(std::string(buf,nr));
    auto x=Codec::Encode(input);
    collector->addPayload(std::string(reinterpret_cast<char*>(&x),sizeof(x)));
}

void Collector::HandlerInterrupt(int fd, void *user_data) {
    auto collector=reinterpret_cast<Collector*>(user_data);
    auto ss=Signal::Read(fd);
    LOG(TAG "receive signal #%d",ss.ssi_signo);
    collector->quit();
}

bool Collector::Connect() {
    ERROR_RETURN((fd_=socket(AF_INET,SOCK_STREAM,0))==-1,false,,1);
    Cleaner/*go defer alike*/ cleaner{[this]{close(fd_);}};
    struct timeval tv{CONNECT_TIMEOUT,0};
    SETSOCKOPT(fd_,SOL_SOCKET,SO_SNDTIMEO,tv,false);
    ERROR_RETURN(connect(fd_,SOCKADDR_EX(server_address_))==-1,false,,1);
    cleaner.cancel();
    LOG(TAG "connect to " SOCKADDR_FMT,SOCKADDR_OF(server_address_));
    return true;
}

void Collector::OnConnect() {
    CALL_START();
    if(state_.load(std::memory_order_acquire)==State::DISCONNECTED){
        LOG(TAG "disarm the timer");
        const struct itimerspec its{};
        timerfd_settime(timer_fd_,0,&its/*disarm timer*/,nullptr);
        Connection c{timer_fd_};
        c.epollUnregister(epoll_handler_);
        c.cancel();
//        for(auto&i:handlers_){
//            if(i->fd()==timer_fd_){
//                i->cancel(); /*avoid closing timer*/
//                handlers_.erase(i);
//                break;
//            }
//        }
        std::shared_ptr<Connection> handler;
        for(auto&i:handlers_){
            if(i->fd()==timer_fd_){
                handler=i;
                break;
            }
        }
        if(handler){
            handlers_.erase(handler);
            handler->cancel();
        }
        LOG(TAG "erase the timer(#%d) from handler list",timer_fd_);
    }
    auto handler=std::make_shared<Connection>(fd_);
    handler->epollRegister(epoll_handler_,EPOLLRDHUP);
    handler->registerCallback({nullptr,nullptr,[handler,this](int&){
        LOG(TAG "remote server is disconnected");
//        handler->cancel(); /*avoid closing client(this endpoint is marked as connected)*/
        handlers_.erase(handler);
        OnDisconnect();
    }},nullptr);
    handlers_.insert(handler);
    retry_count=0;
    state_.store(State::READY,std::memory_order_release);
    CALL_END();
}

void Collector::OnDisconnect() {
    CALL_START();
    const struct itimerspec its{{RECONNECT_DURATION*2,0},{RECONNECT_DURATION,0}};
    LOG(TAG "arm the timer");
    timerfd_settime(timer_fd_,0,&its,nullptr);
    auto handler=std::make_shared<Connection>(timer_fd_);
    handler->epollRegister(epoll_handler_,EPOLLIN);
    handler->registerCallback({[this](int fd,void*user_data){
        unsigned long expiration{};
//        auto collector=reinterpret_cast<Collector*>(user_data);
        read(fd,&expiration,sizeof(expiration));
        LOG(TAG "#%ld try to connect server",(retry_count+=expiration));
        if(Connect()){
            OnConnect();
        }
    },nullptr,nullptr},nullptr);
    handlers_.insert(handler);
    state_.store(State::DISCONNECTED,std::memory_order_release);
    CALL_END();
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
    auto ports=split<short>(argv[1],',',[](const std::string&s)->short{return (std::stoi(s)&/*lower precedence*/0xffu)+PORT_BASE;});
    Collector collector{SERVER_IP,SERVER_PORT};
    collector.registerSource(Signal().registerSignal(SIGINT).registerSignal(SIGQUIT).fd(), &Collector::HandlerInterrupt);
    for(auto port:ports){
        collector.registerSource(Group(GROUP_IP,port).fd()/*transfer ownership*/, &Collector::HandleGroupMsg);
    }
    collector.loop();
//    pause();
//    collector.stop();
}