#ifndef MONITOR_TEST_H
#define MONITOR_TEST_H

#include <signal.h>

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

#define MAIN() int main()
#define MAIN_EX(argc, argv) int main(int argc,char*argv[])

#endif //MONITOR_TEST_H
