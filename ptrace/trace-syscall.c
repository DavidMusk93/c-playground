//
// Created by Steve on 7/9/2020.
// @ref https://www.linuxjournal.com/article/6100
//

#include "common.h"

MAIN() {
    pid_t child;
    long orig_eax;
    child = fork();
    if (child == 0) {
        ptrace(PTRACE_TRACEME, 0, 0, 0);
        execl("/bin/ls", "ls", NULL); /* NULL is different from 0 for compiler */
    } else {
        int status = -1;
        waitpid(child, &status, 0);
        assert(WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP);
        LOG("child status: %x", status);
        orig_eax = ptrace(PTRACE_PEEKUSER, child, USER_REGS_OFF(orig_rax), 0); /* 59 for execve by `ausyscall --dump` */
        LOG("the child made a syscall %ld", orig_eax);
        ptrace(PTRACE_CONT, child, 0, 0);
    }
    return 0;
}