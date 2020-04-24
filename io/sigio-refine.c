//
// Created by Steve on 4/24/2020.
//

#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200112L

#include <signal.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "macro.h"
#include "utility.h"

MAIN() {
    int flags;
    struct termios ori;
    char ch;
    bool done = false;
    siginfo_t si;
    sigset_t ss;
    int REAL_TIME_SIG = SIGRTMIN + 1;
    sigemptyset(&ss);
    sigaddset(&ss, REAL_TIME_SIG);
    ERROR_EXIT(fcntl(STDIN_FILENO, F_SETSIG, REAL_TIME_SIG) == -1 /* Not work in WSL */, "fcntl(F_SETSIG)");
    ERROR_EXIT(fcntl(STDIN_FILENO, F_SETOWN, getpid()) == -1, "fcntl(F_SETOWN)");
    flags = fcntl(STDIN_FILENO, F_GETFL);
    ERROR_EXIT(fcntl(STDIN_FILENO, F_SETFL, flags | O_ASYNC | O_NONBLOCK) == -1, "fcntl(F_SETFL)");
    ERROR_EXIT(tty_set_char_break(STDIN_FILENO, &ori) == -1, "tty_set_char_break");
    sigprocmask(SIG_BLOCK, &ss, 0);
    while (!done) {
        sigwaitinfo(&ss, &si);
        assert(si.si_signo == REAL_TIME_SIG && si.si_fd == STDIN_FILENO && si.si_code == POLL_IN);
        /* Should perform as much I/O (e.g., read as many bytes) as possible. */
        if (read(STDIN_FILENO, &ch, 1) > 0) {
            printf("read %c\n", ch);
            done = ch == '#';
        }
    }
    ERROR_EXIT(tcsetattr(STDIN_FILENO, TCSAFLUSH, &ori) == -1, "tcsetattr");
    return 0;
}