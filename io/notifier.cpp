//
// Created by Steve on 8/11/2020.
//

#include "notifier.h"
#include "codec.h"
#include "reader.h"
#include "tool.h"
#include "secure.h"

Terminator terminator;

class RemoteReader:public Reader{
#define TAG "(RELAY)"
#define SOCKADDR_EX(x) reinterpret_cast<struct sockaddr*>(&x),sizeof(x)
public:
    static constexpr short PORT=10001;
    static constexpr int BACKLOG=5;
    static constexpr int BAD_VALUE=-0xffff;

public:
    RemoteReader():Reader(),fds_{-1,-1},state_(State::WAIT),x_(BAD_VALUE){
        int sock;
        pipe2(fds_,O_NONBLOCK);
        ERROR_RETURN((sock=socket(AF_INET,SOCK_STREAM,0))==-1,,,"socket: %s",ERROR_S);
        Cleaner cleaner{[&sock](){close(sock);}};
        struct sockaddr_in addr={.sin_family=AF_INET,.sin_port=htons(PORT),.sin_addr={INADDR_ANY},};
        int REUSE=1;
        SETSOCKOPT(sock,SOL_SOCKET,SO_REUSEADDR,REUSE,);
        SETSOCKOPT(sock,SOL_SOCKET,SO_REUSEPORT,REUSE,);
        ERROR_RETURN(bind(sock,SOCKADDR_EX(addr))==-1,,,"bind: %s",ERROR_S);
        ERROR_RETURN(listen(sock,BACKLOG)==-1,,,"listen: %s",ERROR_S);
        cleaner.cancel();
        GetFd()=sock;
        actor_=std::thread(&RemoteReader::Run,this);
    }

    ~RemoteReader() override{
        terminator.trigger();
        if(actor_.joinable()){
            actor_.join();
        }
        FdHelper::Close(GetFd());
        FdHelper::Close(fds_[0]);
        FdHelper::Close(fds_[1]);
    }

    bool Eof() override {
        return false;
    }

    short GetShort() override {
        return 0;
    }

    int GetInt() override {
        if(std::atomic_load_explicit(&state_,std::memory_order_relaxed)==State::READY){
            return std::atomic_load_explicit(&x_,std::memory_order_acquire);
        }
        return BAD_VALUE;
    }

    int GetObserveFd() override {
        return fds_[FdHelper::kPipeRead];
    }

protected:
    enum class State{
        READY,
        WAIT,
    };

    void Run(){
        THREAD_KERNEL_START(RELAY);
#define MAX_EVENTS 10
#define TIMEOUT 5000/*ms*/
//        static int count=0;
        struct epoll_event ev{},events[MAX_EVENTS]{};
        int nfds,epollfd;
        ERROR_RETURN((epollfd=epoll_create1(0))==-1,,,"epoll_create1: %s",ERROR_S);
        Cleaner cleaner{[&epollfd](){close(epollfd);}};
        ev.events=EPOLLIN;
        ev.data.fd=GetFd();
        ERROR_RETURN(epoll_ctl(epollfd,EPOLL_CTL_ADD,GetFd(),&ev)==-1,,,"epoll_ctl: %s",ERROR_S);
        ev.events=EPOLLIN;
        ev.data.fd=terminator.observeFd();
        ERROR_RETURN(epoll_ctl(epollfd,EPOLL_CTL_ADD,terminator.observeFd(),&ev)==-1,,,"epoll_ctl: %s",ERROR_S);
        for(;;){
            POLL(nfds,epoll_wait,epollfd,events,MAX_EVENTS,TIMEOUT);
            if(nfds==0){
                std::atomic_store_explicit(&state_,State::WAIT,std::memory_order_relaxed);
            }
            for(int i=0;i<nfds;++i){
                const auto&fd=events[i].data.fd;
                if(fd==GetFd()){ /*accept new connection*/
                    int sock{-1};
                    struct sockaddr_in peer{};
                    socklen_t len{sizeof(peer)};
                    ERROR_RETURN((sock=accept4(fd,(struct sockaddr*)&peer,&len,O_NONBLOCK))==-1,,,"accept4: %s",ERROR_S);
                    Cleaner onfail{[sock]{close(sock);}};
                    LOG(TAG "new connection from %s:%d",inet_ntoa(peer.sin_addr),ntohs(peer.sin_port));
                    ev.events=EPOLLIN|EPOLLET|EPOLLHUP|EPOLLRDHUP;
                    ev.data.fd=sock;
                    LOG(TAG "add %d to epoll instance",sock);
                    ERROR_RETURN(epoll_ctl(epollfd,EPOLL_CTL_ADD,sock,&ev)==-1,,,"epoll_ctl: %s",ERROR_S);
                    onfail.cancel();
                }else if(fd==terminator.observeFd()){ /*terminate*/
                    break;
                }else{ /*enable multiple sources*/
                    LOG(TAG "#%d is ready: %#x",fd,events[i].events);
                    if(events[i].events&EPOLLRDHUP){
                        Cleaner onfinal{[fd]{close(fd);}};
                        LOG(TAG "remote connection (%d) closed",fd);
                        ERROR_RETURN4(epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,nullptr)==-1,,,1);
                    }else if(events[i].events&EPOLLIN){
                        auto buffer=Secure::CreateBuffer();
                        unsigned long x{};
                        recv(fd,&buffer[0],buffer.size(),MSG_WAITALL); /*block read*/
                        if(!Secure::Decrypt(buffer,&x,sizeof(x))){
                            Cleaner onfinal{[fd]{close(fd);}};
                            LOG(TAG "illegal peer,kick out #%d",fd);
                            ERROR_RETURN4(epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,nullptr)==-1,,,1);
                            continue;
                        }
                        if(filter_&&!filter_(Codec::Output::RetrieveTopic(x))){
                            LOG(TAG "no subscriber,drop payload");
                            continue;
                        }
                        int ub=FdHelper::UnreadSize(GetObserveFd());
                        if(ub<128){ /*avoid filling up the pipe*/
                            auto nw=write(fds_[FdHelper::kPipeWrite],&x,sizeof(x));
                            if(nw==-1){
                                LOG(TAG "write failed:%s",ERROR_S);
                            }
                        }else{
                            LOG(TAG "buffer is full,drop payload");
                        }
//                        std::atomic_store_explicit(&x_,x,std::memory_order_release);
                        std::atomic_store_explicit(&state_,State::READY,std::memory_order_release);
                    }
                }
            }
        }
#undef TIMEOUT
#undef MAX_EVENTS
        THREAD_KERNEL_END(RELAY);
    }

public:
    void registerFilter(Filter &&filter) override {
        filter_.swap(filter);
    }

private:
    int fds_[2];
    std::atomic<State> state_;
    std::atomic<int> x_;
    std::thread actor_;
    Filter filter_;
#undef SOCKADDR_EX
#undef TAG
};

Notifier::Notifier(){
//        reader_=std::make_unique<RandomReader>("/dev/urandom");
//        reader_=std::make_unique<RemoteReader>(); //c++14 required
    reader_.reset(new RemoteReader());
    reader_->registerFilter([this](const std::string&topic)->bool{return !manager_[topic].empty();});
    actor_=std::thread(&Notifier::Run,this); //danger if it is initialized in constructor member list
}

Notifier::~Notifier(){
    if(actor_.joinable()){
        terminator.trigger();
        actor_.join();
    }
}

void Notifier::Register(intptr_t key, T &&notify, const std::vector<std::string> &topics){
    Spinlock lock{flag_};
    if(subscriber_.count(key)){
        return;
    }
    auto task=std::make_shared<T>(std::move(notify));
    manager_.subscribe(topics,task);
    subscriber_.insert({key,std::weak_ptr<T>(task)});
    LOG("(NOTIFIER)subscriber count #%ld",subscriber_.size());
}

void Notifier::Run(){
    THREAD_KERNEL_START(NOTIFIER);
    struct pollfd pfd[2]={
            {.fd=terminator.observeFd(),.events=POLLIN},
            {.fd=reader_->GetObserveFd(),.events=POLLIN},
    };
    int nfds{};
    for(;;){
        POLL(nfds,poll,pfd,2,-1);
//            ERROR_RETURN(nfds==-1,,{exit(1);/*force quit*/},"poll: %s",ERROR_S);
//            if(nfds==0){
//                continue;
//            }
        if(pfd[0].revents&POLLIN){
            break;
        }
        auto&fd=pfd[1].fd;
        unsigned long x{};
        read(fd,&x,sizeof(x));
        auto output=Codec::Decode(x);
        std::vector<std::shared_ptr<T>> tasks;
        {
            Spinlock lock(flag_);
            std::vector<intptr_t> invalid;
            for(auto&p:subscriber_){
                if(!p.second.lock()){ /*try promotion*/
//                    subscriber_.erase(p.first);
                    invalid.push_back(p.first);
                }
            }
            for(auto&k:invalid){
                LOG("erase handler %p",reinterpret_cast<void*>(k));
                subscriber_.erase(k);
            }
            auto&ref=manager_[output.topic];
            if(ref.empty()){
                LOG("(NOTIFIER)no subscriber,drop '%s'",output.payload.c_str());
                continue;
            }
            tasks.swap(ref);
        }
        std::vector<std::shared_ptr<T>> valid;
//            j2s(output.payload);
        LOG("(NOTIFIER)task count #%ld",tasks.size());
        for(auto&p:tasks){
            if((*p)(output.payload.data(),output.payload.size())){ /*real publish*/
                valid.push_back(std::move(p));
            }
        }
        if(valid.empty()){
            continue;
        }
        Spinlock lock(flag_);
        auto&ref=manager_[output.topic];
        for(auto&k:valid){
            ref.push_back(std::move(k));
        }
    }
    THREAD_KERNEL_END(NOTIFIER);
}

std::vector<std::string> parseTopic(const std::string&uri){
    std::vector<std::string> res;
    char k[16],v[16];
    auto i=uri.find('?');
    if(i==std::string::npos){
        return {};
    }
    while(++i/*jump &*/<uri.size()){
        int n{};
        sscanf(&uri[i],"%[^=]=%[^&]%n",k,v,&n);
        LOG("@TOPIC %s:%s",k,v);
        res.emplace_back(v);
        i+=n;
    }
    return res;
}

//int main(){
//    std::string uri="ws://192.168.10.101:9001?w1=182&w2=184";
//    for(auto&i:parseTopic(uri)){
//        LOG("%s",i.c_str());
//    }
//    std::string json=R"({"topic":"%s","weight":256,"id":"%s"})";
//    j2s(json);
//    LOG("%s",json.c_str());
//}