#include <unistd.h>
#include <wait.h>

#include "macro.h"

#define PLOG(fmt, ...) LOG("#%d " fmt,getpid(),##__VA_ARGS__)

int x_fork() {
    int pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    return pid;
}

int fork_dont_care() {
    int pid = x_fork();
    if (pid) {
        PLOG("parent, directly return");
        return pid;
    } else if (x_fork()) {
        PLOG("child, exit");
        exit(0);
    } else { /* Who could perceive me? */
        PLOG("grandchild, return");
        return 0;
    }
}

MAIN() {
    int pid, status;
    /* is fork_dont_care generates zombie process */
    if ((pid = fork_dont_care())) {
        PLOG("I am parent");
    } else {
        PLOG("I am grandchild");
        exit(0);
    }
    sleep(60);
    waitpid(pid, &status, 0);
    PLOG("child quit:%d,%d", pid, status);
}