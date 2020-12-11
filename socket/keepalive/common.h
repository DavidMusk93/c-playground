//
// Created by Steve on 11/30/2020.
//

#ifndef C_PLAYGROUND_COMMON_H
#define C_PLAYGROUND_COMMON_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <error.h>
#include <stdlib.h>
#include <poll.h>
#include <time.h>
#include <sys/time.h>

#define MAIN() int main()
#define MAIN_EX(argc,argv) int main(int argc,char*argv[])

#define TIMEVAL struct timeval
static inline TIMEVAL*now(){
    static __thread TIMEVAL tv;
    gettimeofday(&tv,0);
    return &tv;
}

typedef const char*(time_format)(TIMEVAL*tv,char buf[64]);
static inline const char*nowstr(time_format*fn,char buf[64]){
    static __thread char innerbuf[64];
    char*p=buf?:innerbuf;
    return fn(now(),p);
}

static inline const char*simple_format(TIMEVAL*tv,char buf[64]){
    sprintf(buf,"%d.%06d",(int)tv->tv_sec,(int)tv->tv_usec);
    return buf;
}

#define LOG(fmt,...) printf("%s " fmt "\n",nowstr(&simple_format,0),##__VA_ARGS__)

#endif //C_PLAYGROUND_COMMON_H
