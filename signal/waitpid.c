//
// Created by Steve on 4/21/2020.
//

#include <unistd.h>
#include <sys/wait.h>

#include "macro.h"

static void foo() {
#define VALUE 13 /* macro lifetime until #undef */
    log("@%s %d", __func__, VALUE);
#undef VALUE
}

static void bar() {
#define VALUE "yes"
    log("@%s %s", __func__, VALUE);
#undef VALUE
}

int main() {
    switch (fork()) {
        case -1: {
            log("fork: %m");
            return 1;
        }
        case 0: {
            log("subprocess: %ld", (long) getpid());
//            pause();
            exit(15);
        }
        default: {
            int status = -1;
            wait(&status);
            log("child terminate status: %#x", status);
        }
    }
    foo(), bar();
    return 0;
}