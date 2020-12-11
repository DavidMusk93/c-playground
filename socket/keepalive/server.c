//
// Created by Steve on 11/30/2020.
//

#include "common.h"

typedef struct{
    int fd;
    int keepidle;
    int keepintvl;
    int keepcnt;
}keepalive_context_t;

void*keepalive_context_dump(keepalive_context_t*ctx){
    int optval=0;
    socklen_t optlen=sizeof(optval);
    getsockopt(ctx->fd,SOL_SOCKET,SO_KEEPALIVE,&optval,&optlen);
    if(optval){
        getsockopt(ctx->fd,SOL_TCP,TCP_KEEPIDLE,&ctx->keepidle,&optlen);
        getsockopt(ctx->fd,SOL_TCP,TCP_KEEPINTVL,&ctx->keepintvl,&optlen);
        getsockopt(ctx->fd,SOL_TCP,TCP_KEEPCNT,&ctx->keepcnt,&optlen);
        return ctx;
    }
    return 0;
}

void keepalive_context_enable(const keepalive_context_t*ctx){
    int optval=1;
    setsockopt(ctx->fd,SOL_SOCKET,SO_KEEPALIVE,&optval,sizeof(int));
    setsockopt(ctx->fd,SOL_TCP,TCP_KEEPIDLE,&ctx->keepidle,sizeof(int));
    setsockopt(ctx->fd,SOL_TCP,TCP_KEEPINTVL,&ctx->keepintvl,sizeof(int));
    setsockopt(ctx->fd,SOL_TCP,TCP_KEEPCNT,&ctx->keepcnt,sizeof(int));
}

int create_server(short port){
    int optval=1;
    int fd=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr={.sin_family=AF_INET,.sin_port=htons(port),.sin_addr={INADDR_ANY}};
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval));
    if(bind(fd,(struct sockaddr*)&addr,sizeof(addr))!=-1&&listen(fd,1)!=-1){
        return fd;
    }
    perror("fail to create server");
    close(fd);
    return -1;
}

void handle_connection(int fd,int timeout){
    keepalive_context_t ctx={.fd=fd,.keepidle=5,.keepintvl=3,.keepcnt=3};
    keepalive_context_enable(&ctx);
    memset(&ctx,0,sizeof(ctx));
    if(keepalive_context_dump(&ctx)){
        LOG("@KEEPALIVE idle:%d,intvl:%d,cnt:%d",ctx.keepidle,ctx.keepintvl,ctx.keepcnt);
    }
    struct pollfd pfd;
    memset(&pfd,0,sizeof(pfd));
    pfd.fd=fd;
    pfd.events=POLLIN;
    ssize_t nfds;
    char buf[1024];
    while((nfds=poll(&pfd,1,timeout))!=-1){
        if(nfds==0){
            LOG("@HANDLE timeout");
        }else{
            LOG("%x",pfd.revents);
            if(pfd.revents&POLLIN){
                int nr=read(fd,buf,sizeof(buf));
                if(!nr){
                    break;
                }
                LOG("@RECV \"%.*s\"",nr,buf);
                write(fd,buf,nr);
            }else if(pfd.revents&POLLHUP){
                LOG("@PEER remote connection closed");
                break;
            }
        }
    }
    close(fd);
}

MAIN_EX(argc,argv){
    short port=6666;
    if(argc>1){
        port=atoi(argv[1])&0xffff;
    }
    int sfd=create_server(port);
    if(sfd==-1){
        return 1;
    }
    for(;;){
        struct sockaddr_in peer;
        socklen_t peerlen=sizeof(peer);
        memset(&peer,0,peerlen);
        int fd=accept(sfd,(struct sockaddr*)&peer,&peerlen);
        LOG("@MAIN new connection from %s:%d",inet_ntoa(peer.sin_addr),ntohs(peer.sin_port));
        handle_connection(fd,1000);
    }
}