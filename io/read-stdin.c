//
// Created by Steve on 8/20/2020.
//

#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "macro.h"

#define NREAD(fd) ({\
    int _s=0;\
    ioctl(fd,FIONREAD,&_s);\
    _s;\
})

MAIN(){
    int flags=0;
//    fcntl(STDIN_FILENO,F_GETFL,&flags);
//    LOG("flags:%x",flags);
    struct pollfd pfd={.fd=STDIN_FILENO,.events=POLLIN};
    /* A file descriptor is considered ready if it is possible to perform
     * a corresponding I/O operation without blocking (@stackoverflow 43499743). */
    while(poll(&pfd,1,-1)>0){
        if(pfd.revents&POLLIN){
            LOG("left bytes: %d",NREAD(STDIN_FILENO));
            char buf[64]={};
            fgets(buf,sizeof(buf),stdin); /*has an inner buffer*/
//            char c;
//            char*p=buf;
//            while(read(pfd.fd,&c,1)==1){ /*raed one by one(less efficient)*/
//                if(c==NL){
//                    break;
//                }
//                *p++=c;
//            }
            if(!*buf){
                break;
            }
            LOG("@INPUT '%s'",buf);
        }
    }
    return 0;
}