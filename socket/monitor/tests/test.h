#ifndef MONITOR_TEST_H
#define MONITOR_TEST_H

#include <signal.h>
#include <stdio.h>

#define SETNOBUF() \
setbuf(stdout,nullptr);\
setbuf(stderr,nullptr)

#define SIGBLOCK() \
sigset_t __ss{};\
int __sig{};\
sigaddset(&__ss,SIGINT);\
sigaddset(&__ss,SIGQUIT);\
sigwait(&__ss,&__sig)

#define INSTALLSIGINTHANDLER(x) \
struct sigaction __sa{};\
sigemptyset(&__sa.sa_mask);\
__sa.sa_handler=x;\
sigaction(SIGINT,&__sa,nullptr)

#define DECLAREPOLLSIGNALHANDLER(__global_handler, __fn) \
static void*__global_handler;\
static void __fn(int sig){\
    if(__global_handler){\
        reinterpret_cast<sun::io::Poll*>(__global_handler)->quit();\
    }\
}

#define MAIN() int main()
#define MAIN_EX(argc, argv) int main(int argc,char*argv[])

#endif //MONITOR_TEST_H
