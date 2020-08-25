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

#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <poll.h>

#define LOG(fmt,...) printf(fmt "\n",##__VA_ARGS__)

#define ERROR_RETURN(error,code,block,fmt,...) \
do{\
    if(error){\
        LOG(fmt,##__VA_ARGS__);\
        block\
        return code;\
    }\
}while(0)

#define ERROR_RETURN4(expr,code,cleanup,err) \
do{\
    if(expr){\
        cleanup\
        LOG("%s: %s",#expr,sizeof(#err)==1?"":ERROR_S);\
        return code;\
    }\
}while(0)

#define SETSOCKOPT(sock,level,name,val,code) \
ERROR_RETURN(\
setsockopt(sock,level,name,&val,sizeof(val))!=0,\
code,\
{close(sock);sock=-1;},\
"setsockopt(" #name ") error: %s",ERROR_S)

#define ERROR_S strerror(errno)

class Reader{
public:
    Reader():fd_(-1){}
    virtual ~Reader()=default;

    virtual bool Eof()=0;
    virtual short GetShort()=0;
    virtual int GetInt()=0;
    virtual int GetObserveFd()=0;

protected:
    int&GetFd(){
        return fd_;
    }

private:
    int fd_;
};

class FdHelper{
public:
    FdHelper()=delete;

    static void SetNonblock(int fd){
        int flags;
        fcntl(fd,F_GETFL,&flags);
        fcntl(fd,F_SETFL,flags|O_NONBLOCK);
    }

    static int UnreadSize(int fd){
        int size{};
        ioctl(fd,FIONREAD,&size);
        return size;
    }

public:
    static constexpr int kPipeRead=0;
    static constexpr int kPipeWrite=1;
};

class Terminator{
public:
    Terminator():fds_{-1,-1}{
        pipe2(fds_,O_NONBLOCK);
    }

    ~Terminator(){
        close(fds_[0]),close(fds_[1]);
        fds_[0]=fds_[1]=-1;
    }

    int GetObserveFd(){
        return fds_[FdHelper::kPipeRead];
    }

    void Trigger(){
        char c='Q';
        write(fds_[FdHelper::kPipeWrite],&c,1);
    }

    void Cleanup(){
        char c;
        read(fds_[FdHelper::kPipeRead],&c,1);
    }

private:
    int fds_[2];
};
Terminator terminator;

using Closure=std::function<void()>;

class Cleaner{
public:
    Cleaner(Closure closure):final_(std::move(closure)){}
    ~Cleaner(){
        if(final_){
            final_();
        }
    }

    void Cancel(){
        Closure trivial;
        final_.swap(trivial);
    }

private:
    Closure final_;
};

class RemoteReader:public Reader{
#define SOCKADDR_EX(x) reinterpret_cast<struct sockaddr*>(&x),sizeof(x)
public:
    static constexpr short PORT=10001;
    static constexpr int BACKLOG=1;
    static constexpr int BAD_VALUE=-0xffff;

public:
    RemoteReader():Reader(),fds_{-1,-1},state_(State::WAIT),x_(BAD_VALUE){
        int sock;
        pipe2(fds_,O_NONBLOCK);
        ERROR_RETURN((sock=socket(AF_INET,SOCK_STREAM,0))==-1,,,"socket: %s",ERROR_S);
        Cleaner cleaner{[&sock](){
            if(sock!=-1){
                close(sock);
                sock=-1;
            }
        }};
        struct sockaddr_in addr={.sin_family=AF_INET,.sin_port=htons(PORT),.sin_addr={INADDR_ANY},};
        int REUSE=1;
        SETSOCKOPT(sock,SOL_SOCKET,SO_REUSEADDR,REUSE,);
        SETSOCKOPT(sock,SOL_SOCKET,SO_REUSEPORT,REUSE,);
        ERROR_RETURN(bind(sock,SOCKADDR_EX(addr))==-1,,,"bind: %s",ERROR_S);
        ERROR_RETURN(listen(sock,BACKLOG)==-1,,,"listen: %s",ERROR_S);
        cleaner.Cancel();
        GetFd()=sock;
        actor_=std::thread(&RemoteReader::Run,this);
    }

    ~RemoteReader() override{
        terminator.Trigger();
        if(actor_.joinable()){
            actor_.join();
        }
        if(GetFd()!=-1){
            close(GetFd());
        }
        close(fds_[0]),close(fds_[1]);
        fds_[0]=fds_[1]=-1;
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
        static int count=0;
        struct epoll_event ev{},events[MAX_EVENTS]{};
        int nfds,epollfd,conn_sock;
        Cleaner cleaner{[&epollfd](){
                if(epollfd!=-1){
                    close(epollfd);
                }
        }};
        ERROR_RETURN((epollfd=epoll_create1(0))==-1,,,"epoll_create1: %s",ERROR_S);
        ev.events=EPOLLIN;
        ev.data.fd=GetFd();
        ERROR_RETURN(epoll_ctl(epollfd,EPOLL_CTL_ADD,GetFd(),&ev)==-1,,,"epoll_ctl: %s",ERROR_S);
        ev.events=EPOLLIN;
        ev.data.fd=terminator.GetObserveFd();
        ERROR_RETURN(epoll_ctl(epollfd,EPOLL_CTL_ADD,terminator.GetObserveFd(),&ev)==-1,,,"epoll_ctl: %s",ERROR_S);
        for(;;){
            ERROR_RETURN((nfds=epoll_wait(epollfd,events,MAX_EVENTS,TIMEOUT))==-1,,,"epoll_wait: %s",ERROR_S);
            if(nfds==0){
                LOG("epoll timeout");
                if(++count==3){
                    continue;
                }
                count=0;
                std::atomic_store_explicit(&state_,State::WAIT,std::memory_order_relaxed);
            }
            for(int i=0;i<nfds;++i){
                const auto&fd=events[i].data.fd;
                if(fd==GetFd()){
                    struct sockaddr_in peer{};
                    socklen_t len{};
                    ERROR_RETURN((conn_sock=accept4(fd,(struct sockaddr*)&peer,&len,O_NONBLOCK))==-1,,,"accept4: %s",ERROR_S);
                    LOG("new connection from %s:%d",inet_ntoa(peer.sin_addr),ntohs(peer.sin_port));
                    ev.events=EPOLLIN|EPOLLET|EPOLLHUP|EPOLLRDHUP;
                    ev.data.fd=conn_sock;
                    LOG("add %d to epoll instance",conn_sock);
                    ERROR_RETURN(epoll_ctl(epollfd,EPOLL_CTL_ADD,conn_sock,&ev)==-1,,{close(conn_sock);},"epoll_ctl: %s",ERROR_S);
                }else if(fd==terminator.GetObserveFd()){
                    break;
                }else if(fd==conn_sock){
                    LOG("%d is ready: %x",conn_sock,events[i].events);
                    if(events[i].events&EPOLLRDHUP){
                        LOG("remote connection (%d) closed",conn_sock);
                        ev.data.fd=conn_sock;
//                        ERROR_RETURN(epoll_ctl(epollfd,EPOLL_CTL_DEL,conn_sock,&ev)==-1,,,"epoll_ctl: %s",ERROR_S);
                        ERROR_RETURN4(epoll_ctl(epollfd,EPOLL_CTL_DEL,conn_sock,&ev)==-1,,,1);
                        close(conn_sock),conn_sock=-1;
                    }else if(events[i].events&EPOLLIN){
                        int x{};
                        read(conn_sock,&x,sizeof(x));
                        if(FdHelper::UnreadSize(GetObserveFd())<128){ //avoid filling up pipe
                            write(fds_[FdHelper::kPipeWrite],&x,sizeof(x));
                        }
                        std::atomic_store_explicit(&x_,x,std::memory_order_release);
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

class RandomReader:public Reader{
public:
    RandomReader(const std::string&file):Reader(),i_(kBufferSize){
        int fd=-1;
        ERROR_RETURN((fd=open(file.c_str(),O_RDONLY))==-1,,,"fail to open %s: %s",file.c_str(),strerror(errno));
        GetFd()=fd;
    }

    ~RandomReader() override{
        close(GetFd());
    }

    bool Eof() override {
        return false;
    }

    short GetShort() override {
        short x{};
        Fetch(&x,sizeof x);
        return x;
    }

    int GetInt() override {
        return 0;
    }

    int GetObserveFd() override {
        return 0;
    }

protected:
    void Fetch(void *data,size_t len){
        if(len){
            auto p=reinterpret_cast<char*>(data);
            while(len--){
                if(i_+1>kBufferSize){
                    i_=0;
                    read(GetFd(),buf_,kBufferSize);
                }
                *p++=buf_[i_++];
            }
        }
    }
private:
    int i_;
    char buf_[1024];

    static constexpr int kBufferSize=sizeof(buf_);
};

//class AtomicFlagFactory{
//public:
//    static std::atomic_flag Create();
//};
//
//std::atomic_flag AtomicFlagFactory::Create() {
//    return ATOMIC_FLAG_INIT;
//}

class SpinLock{
public:
    SpinLock(std::atomic_flag&flag):flag_(flag){
        while(flag_.test_and_set(std::memory_order_acquire));
    }

    ~SpinLock(){
        flag_.clear(std::memory_order_release);
    }

private:
    std::atomic_flag&flag_;
};

class Notifier{
public:
    using T=std::function<bool(const void*,size_t)>;

    Notifier(){
//        reader_=std::make_unique<RandomReader>("/dev/urandom");
//        reader_=std::make_unique<RemoteReader>(); //c++14 required
        reader_.reset(new RemoteReader());
        actor_=std::thread(&Notifier::Run,this); //danger if it is initialized in constructor member list
    };

    ~Notifier(){
        if(actor_.joinable()){
            terminator.Trigger();
            actor_.join();
        }
    }

    void Register(intptr_t key,T&& notify){
//        if(actor_.joinable()){
//            Terminate();
//            actor_.join();
//            CleanupTerminator();
//        }
        SpinLock lock{flag_};
        if(callback_.count(key)){
            return;
        }
        callback_.insert({key,std::move(notify)});
//        notify_=std::move(notify);
    }

protected:
    void Run(){
        struct pollfd pfd[2]={
                {.fd=terminator.GetObserveFd(),.events=POLLIN},
                {.fd=reader_->GetObserveFd(),.events=POLLIN},
        };
        char buf[16];
        for(;;){
            int nfds=poll(pfd,2,100);
            ERROR_RETURN(nfds==-1,,,"poll: %s",ERROR_S);
            if(nfds==0){
                continue;
            }
            if(pfd[0].revents&POLLIN){
                break;
            }
            auto&fd=pfd[1].fd;
            decltype(callback_) tasks;
            {
                SpinLock lock(flag_);
                if(callback_.empty()){
                    continue;
                }
                tasks=callback_;
            }
            //random reader
//            short x=50;
//            x+=reader_->GetShort()&0xf;
            //check atomic variable (bad)
//            short x=reader_->GetShort();
//            if(x==RemoteReader::BAD_VALUE){
//                LOG("source not ready");
//                continue;
//            }
            int x;
            read(fd,&x,sizeof(x));
            int n=sprintf(buf,"%d",x);
            std::unordered_set<intptr_t> invalid;
            for(auto&p:tasks){
                if(!p.second(buf,n)){
                    invalid.insert(p.first);
                }
            }
            SpinLock lock(flag_);
            for(auto&k:invalid){
                callback_.erase(k);
            }
        }
//        CleanupTerminator();
    }

private:
//    RandomReader rand_{"/dev/urandom"};
    std::unique_ptr<Reader> reader_;
//    T notify_;
    std::unordered_map<intptr_t,T> callback_;
    std::thread actor_;
    std::atomic_flag flag_=ATOMIC_FLAG_INIT;
};

#undef ERROR_S
#undef ERROR_RETURN
#undef LOG

#include "macro.h"

MAIN(){
    RandomReader reader{"/dev/urandom"};
    int n=10;
    while(n--){
        LOG("%d",reader.GetShort());
    }
    auto task=[](const void*data,size_t len)->bool{
        static int count=0;
        LOG("%d-th %.*s",++count,(int)len,reinterpret_cast<const char*>(data));
        return true;
    };
//    Notifier().Register(task); //deconstructor on done
    Notifier notifier;
    notifier.Register(0,task);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    notifier.Register(1,task);
    std::this_thread::sleep_for(std::chrono::seconds(5));
}