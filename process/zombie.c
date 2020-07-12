//
// Created by Steve on 7/12/2020.
//

#include <unistd.h>

#include "macro.h"

MAIN() {
    pid_t child;
    /** ZOMBIE PROCESS (@ref https://man7.org/linux/man-pages/man5/proc.5.html)
     * cat /proc/$child/status
     *     State: Z (zombie)
     *     Tgid:  23876 (thread group ID)
     *     Ngid:  0 (NUMA group ID)
     *     Pid:   24876 (thread ID)
     *     PPid:  23875 (PID of parent process)
     */
    switch ((child = fork())) {
        case -1: {
            LOG("fork: %m");
            exit(1);
        }
        case 0: {
            LOG("(CHILD)pid: %d", (int) getpid());
            exit(0);
        }
        default: {
            LOG("(PARENT)child pid: %d", (int) child);
            pause();
        }
    }
}