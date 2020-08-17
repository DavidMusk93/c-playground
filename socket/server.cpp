//
// Created by Steve on 8/17/2020.
//

#include "network.h"

Server::Server(Type type, Config config,Terminator&terminator):terminator_(terminator) {
    if(type==Type::TCP){
        int fd;
        ERROR_RETURN((fd=socket(AF_INET,SOCK_STREAM,0))==-1,,,1);
        Cleaner cleaner{[&fd](){close(fd);}};
        MAKE_SOCKADDR_IN(local,INADDR_ANY,htons(config.port));
        int REUSE=1;
        SETSOCKOPT(fd,SOL_SOCKET,SO_REUSEADDR,REUSE,);
        SETSOCKOPT(fd,SOL_SOCKET,SO_REUSEPORT,REUSE,);
        ERROR_RETURN(bind(fd,SOCKADDR_EX(local))==-1,,,1);
        //backlog specifies the queue length for completely established sockets waiting to be accepted
        ERROR_RETURN(listen(fd,config.backlog)==-1,,,1);
        cleaner.cancel();
        server_handler_=std::make_shared<Connection>(fd);
        actor_=std::thread(&Server::run,this);
    }else{
        throw std::logic_error("UDP is not currently supported");
    }
}

Server::~Server() {
    terminator_.trigger();
}

void Server::run() {
#define MAX_EVENTS 10
#define TIMEOUT 1000
    struct epoll_event events[MAX_EVENTS];
    int nfds,epoll_handler,sock;
    ERROR_RETURN((epoll_handler=epoll_create1(0))==-1,,,1);
    Cleaner cleaner{[&epoll_handler](){close(epoll_handler);}};
    ERROR_RETURN(server_handler_->epollRegister(epoll_handler,EPOLLIN)==-1,,,0);
    server_handler_->registerCallback({&Server::Accept,nullptr,nullptr},this);
    connections_.insert(server_handler_);
//    struct epoll_event ev{};
//    ev.events=EPOLLIN;
//    ev.data.fd=terminator_.observeFd();
    Connection term(terminator_.observeFd());
    ERROR_RETURN(term.epollRegister(epoll_handler,EPOLLIN)==-1,,,0);
    term.cancel();
    for(;;){
        ERROR_RETURN((nfds=epoll_wait(epoll_handler,events,MAX_EVENTS,TIMEOUT))==-1,,,1);
        if(nfds==0){
            LOG("epoll timeout,interest list size: %d",(int)connections_.size());
        }
        for(int i=0;i<nfds;++i){
            const auto&fd=events[i].data.fd;
            if(fd==terminator_.observeFd()){
                break;
            }else{
                for(auto&handler:connections_){
                    if(handler->fd()==fd){
                        handler->eventTrigger(epoll_handler,events[i].events);
                        if(new_connection_){
                            auto connection=new_connection_;
                            ERROR_RETURN(new_connection_->epollRegister(epoll_handler,EPOLLIN|EPOLLET|EPOLLRDHUP)==-1,,,0);
                            auto p=connection.get();
                            new_connection_->registerCallback({[p](int fd,void*user_data){
//                                struct sockaddr_in peer{};
//                                socklen_t len=sizeof(peer);
                                char buf[1024];
//                                int nr=recvfrom(fd,buf,sizeof(buf),0,(struct sockaddr*)&peer,&len);
                                int nr=recv(fd,buf,sizeof(buf),0);
                                LOG("(%p)received '%.*s'",p,nr,buf);
                            }
                            ,nullptr
                            ,[connection,this](int&fd){
                                LOG("(%p) connection closed",connection.get());
                                connections_.erase(connection);
                            }},nullptr);
                            connections_.emplace(std::move(new_connection_));
                        }
                        break;
                    }
                }
            }
        }
    }
#undef TIMEOUT
#undef MAX_EVENTS
}

void Server::Accept(int server_fd, void *user_data) {
    auto server=reinterpret_cast<Server*>(user_data);
    struct sockaddr_in peer{};
    socklen_t len=sizeof(peer);
    int fd;
    ERROR_RETURN((fd=accept4(server_fd,(struct sockaddr*)&peer,&len,O_NONBLOCK))==-1,,,1);
    server->new_connection_=std::make_shared<Connection>(fd);
    LOG("(%p)new connection from " SOCKADDR_FMT,server->new_connection_.get(),SOCKADDR_OF(peer));
//    server->connections_.insert(std::make_shared<Connection>(fd));
}

int main(int argc,char*argv[]){
#define PORT_BASE 10000
    short port;
    if(argc==1){
        port=PORT_BASE+2;
    }else{
        long t=strtol(argv[1],nullptr,10);
        if(t>PORT_BASE){
            t%=PORT_BASE;
        }
        port=PORT_BASE+t;
    }
    Terminator terminator;
    LOG("bind server to 0.0.0.0:%d",port);
    Server server(Server::Type::TCP,Server::Config{.port=port,.backlog=1},terminator);
    pause();
#undef PORT_BASE
}