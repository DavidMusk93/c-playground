//
// Created by Steve on 8/17/2020.
//

#ifndef C4FUN_NETWORK_H
#define C4FUN_NETWORK_H

#include <string>
#include <functional>
#include <memory>
#include <thread>
#include <unordered_set>

#include <stdio.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <error.h>
#include <string.h>
#include <poll.h>
#include <fcntl.h>

//#include "time/now.h"

#define LOG(fmt,...) printf(fmt "\n",##__VA_ARGS__)
#define WHERE_FALSE while(0)

#define ERROR_S strerror(errno)
#define ERROR_RETURN(expr,code,cleanup,err) \
do{\
    if(expr){\
        cleanup\
        LOG("%s%s",#expr,err?(": "+std::string(ERROR_S)).c_str():"");\
        return code;\
    }\
}WHERE_FALSE

#define SETSOCKOPT(sock,level,name,val,code) \
ERROR_RETURN(setsockopt(sock,level,name,&val,sizeof(val))!=0,code,{close(sock);sock=-1;},true)

#define SOCKADDR_EX(x) (struct sockaddr*)&x,sizeof(x)
#define SOCKADDR_FMT "%s:%d"
#define SOCKADDR_OF(x) inet_ntoa((x).sin_addr),ntohs((x).sin_port)

#define MAKE_SOCKADDR_IN(name,addr,port) \
struct sockaddr_in name{};\
name.sin_family=AF_INET;\
name.sin_addr.s_addr=addr;\
name.sin_port=port

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

    int observeFd(){
        return fds_[FdHelper::kPipeRead];
    }

    void trigger(){
        char c='Q';
        write(fds_[FdHelper::kPipeWrite],&c,1);
    }

    void cleanup(){
        char c;
        read(fds_[FdHelper::kPipeRead],&c,1);
    }

private:
    int fds_[2];
};

using Closure=std::function<void()>;

class Cleaner{
public:
    Cleaner(Closure closure):final_(std::move(closure)){}
    ~Cleaner(){
        if(final_){
            final_();
        }
    }

    void cancel(){
        Closure trivial;
        final_.swap(trivial);
    }

private:
    Closure final_;
};

class Connection{
public:
    struct Callback{
        std::function<void(int,void*)> on_recv;
        std::function<void(int,void*)> on_send;
        std::function<void(int&)> on_close;
    };
//    using Callback=std::function<void(int,void*)>;
    using Handler=std::shared_ptr<Connection>;
    Connection():Connection(-1){}
    Connection(int fd):fd_(fd){}
    ~Connection(){
        Cleanup(fd_);
    }
    int fd() const;
    int epollRegister(int epoll_handler,int events) const;
    int epollUnregister(int epoll_handler);
    void registerCallback(Callback callback,void*user_data);
    void eventTrigger(int epoll_handler,int events);
    void cancel(){
        fd_=-1;
    }

    static void Cleanup(int&fd){
        if(fd!=-1){
            close(fd);
            fd=-1;
        }
    }

private:
    int fd_;
    Callback callback_;
    void*user_data_;
};

class Server{
public:
    enum class Type:char{
        TCP,
        UDP,
    };
    struct Config{
        const char*ip;
        short port;
        int backlog;
    };
    Server(Type type,Config config,Terminator&terminator);
    ~Server();

protected:
    void run();

    static void Accept(int fd,void*user_data);

private:
    Connection::Handler server_handler_;
    std::thread actor_;
    Terminator&terminator_;
    std::unordered_set<Connection::Handler> connections_;
    Connection::Handler new_connection_;
};

#endif //C4FUN_NETWORK_H
