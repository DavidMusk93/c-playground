//
// Created by Steve on 7/12/2020.
//

#include <unistd.h>

#include "macro.h"

MAIN() {
    pid_t child;
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