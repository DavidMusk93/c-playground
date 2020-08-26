//
// Created by Steve on 8/11/2020.
//

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <vector>

#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <poll.h>

#include "codec.h"
#include "reader.h"
#include "tool.h"

Terminator terminator;

class RemoteReader:public Reader{
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
#define MAX_EVENTS 10
#define TIMEOUT 500 //ms
//        static int count=0;
        struct epoll_event ev{},events[MAX_EVENTS]{};
        int nfds,epollfd,conn_sock;
        ERROR_RETURN((epollfd=epoll_create1(0))==-1,,,"epoll_create1: %s",ERROR_S);
        Cleaner cleaner{[&epollfd](){close(epollfd);}};
        ev.events=EPOLLIN;
        ev.data.fd=GetFd();
        ERROR_RETURN(epoll_ctl(epollfd,EPOLL_CTL_ADD,GetFd(),&ev)==-1,,,"epoll_ctl: %s",ERROR_S);
        ev.events=EPOLLIN;
        ev.data.fd=terminator.observeFd();
        ERROR_RETURN(epoll_ctl(epollfd,EPOLL_CTL_ADD,terminator.observeFd(),&ev)==-1,,,"epoll_ctl: %s",ERROR_S);
        for(;;){
            ERROR_RETURN((nfds=epoll_wait(epollfd,events,MAX_EVENTS,TIMEOUT))==-1,,,"epoll_wait: %s",ERROR_S);
            if(nfds==0){
                std::atomic_store_explicit(&state_,State::WAIT,std::memory_order_relaxed);
            }
            for(int i=0;i<nfds;++i){
                const auto&fd=events[i].data.fd;
                if(fd==GetFd()){
                    struct sockaddr_in peer{};
                    socklen_t len{sizeof(peer)};
                    ERROR_RETURN((conn_sock=accept4(fd,(struct sockaddr*)&peer,&len,O_NONBLOCK))==-1,,,"accept4: %s",ERROR_S);
                    LOG("new connection from %s:%d",inet_ntoa(peer.sin_addr),ntohs(peer.sin_port));
                    ev.events=EPOLLIN|EPOLLET|EPOLLHUP|EPOLLRDHUP;
                    ev.data.fd=conn_sock;
                    LOG("add %d to epoll instance",conn_sock);
                    ERROR_RETURN(epoll_ctl(epollfd,EPOLL_CTL_ADD,conn_sock,&ev)==-1,,{close(conn_sock);},"epoll_ctl: %s",ERROR_S);
                }else if(fd==terminator.observeFd()){
                    break;
                }else if(fd==conn_sock){
                    LOG("#%d is ready: %x",conn_sock,events[i].events);
                    if(events[i].events&EPOLLRDHUP){
                        LOG("remote connection (%d) closed",conn_sock);
                        ev.data.fd=conn_sock;
                        ERROR_RETURN4(epoll_ctl(epollfd,EPOLL_CTL_DEL,conn_sock,&ev)==-1,,,1);
                        close(conn_sock),conn_sock=-1;
                    }else if(events[i].events&EPOLLIN){
                        unsigned long x{};
                        read(conn_sock,&x,sizeof(x));
                        if(FdHelper::UnreadSize(GetObserveFd())<128){ //avoid filling up pipe
                            write(fds_[FdHelper::kPipeWrite],&x,sizeof(x));
                        }
//                        std::atomic_store_explicit(&x_,x,std::memory_order_release);
                        std::atomic_store_explicit(&state_,State::READY,std::memory_order_release);
                    }
                }
            }
        }
#undef TIMEOUT
#undef MAX_EVENTS
    }

private:
    int fds_[2];
    std::atomic<State> state_;
    std::atomic<int> x_;
    std::thread actor_;
#undef SOCKADDR_EX
};

class TopicManager{
public:
    using consumer_t=std::function<bool(const void*,size_t)>;
    using consumer_ptr=std::shared_ptr<consumer_t>;

    void subscribe(const std::vector<std::string>&topics,const consumer_ptr&ptr){
        for(auto&topic:topics){
            table_[topic].push_back(ptr);
        }
    }

    void publish(const std::string&topic,void*payload,size_t len){ //not thread safe
        std::vector<consumer_ptr> t;
        for(auto&fn:table_[topic]){
            if((*fn)(payload,len)){
                t.push_back(std::move(fn));
            }
        }
        table_[topic]=t;
    }

    std::vector<consumer_ptr>&operator[](const std::string&topic){
        return table_[topic];
    }

private:
    std::unordered_map<std::string,std::vector<consumer_ptr>> table_;
};

class Notifier{
public:
    using T=TopicManager::consumer_t;

    Notifier(){
//        reader_=std::make_unique<RandomReader>("/dev/urandom");
//        reader_=std::make_unique<RemoteReader>(); //c++14 required
        reader_.reset(new RemoteReader());
        actor_=std::thread(&Notifier::Run,this); //danger if it is initialized in constructor member list
    };

    ~Notifier(){
        if(actor_.joinable()){
            terminator.trigger();
            actor_.join();
        }
    }

    void Register(intptr_t key,T&& notify,const std::vector<std::string>&topics){
        Spinlock lock{flag_};
        if(subscriber_.count(key)){
            return;
        }
        auto task=std::make_shared<T>(std::move(notify));
        manager_.subscribe(topics,task);
        subscriber_.insert({key,std::weak_ptr<T>(task)});
    }

protected:
    void Run(){
        struct pollfd pfd[2]={
                {.fd=terminator.observeFd(),.events=POLLIN},
                {.fd=reader_->GetObserveFd(),.events=POLLIN},
        };
        for(;;){
            int nfds=poll(pfd,2,-1);
            ERROR_RETURN(nfds==-1,,,"poll: %s",ERROR_S);
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
                auto&ref=manager_[output.topic];
                if(ref.empty()){
                    continue;
                }
                tasks.swap(ref);
            }
            std::vector<std::shared_ptr<T>> valid;
            for(auto&p:tasks){
                if((*p)(output.payload.data(),output.payload.size())){
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
            for(auto&p:subscriber_){
                if(!p.second.lock()){
                    subscriber_.erase(p.first);
                }
            }
        }
    }

private:
//    RandomReader rand_{"/dev/urandom"};
    std::unique_ptr<Reader> reader_;
    std::unordered_map<intptr_t,std::weak_ptr<T>> subscriber_;
//    std::unordered_map<intptr_t,T> callback_;
    TopicManager manager_;
    std::thread actor_;
    std::atomic_flag flag_=ATOMIC_FLAG_INIT;
};

static std::vector<std::string> parseTopic(const std::string&uri){
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

int main(){
    std::string uri="ws://192.168.10.101:9001?w1=182&w2=184";
    for(auto&i:parseTopic(uri)){
        LOG("%s",i.c_str());
    }
}