//
// Created by Steve on 4/24/2020.
//

#include <signal.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>

#include "macro.h"
#include "utility.h"

static volatile sig_atomic_t got_sigio = 0;

static void sigio_handler(int sig) {
    got_sigio = 1;
}

#ifdef TEST_SIGIO

MAIN_EX(argc, argv) {
    int flags, cnt;
    struct termios ori;
    char ch;
    struct sigaction sa;
    bool done;

    /* 1. Establish a handler for the signal delivered by the signal-driven I/O mechanism. */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = sigio_handler;
    ERROR_EXIT(sigaction(SIGIO, &sa, 0) == -1, "sigaction");

    /* 2. Set the owner of the file descriptor. */
    ERROR_EXIT(fcntl(STDIN_FILENO, F_SETOWN, getpid()) == -1, "fcntl(F_SETOWN)");

    /* 3. Enable nonblocking I/O; 4. Enable signal-driven I/O by turning on the O_ASYNC open file status flag. */
    flags = fcntl(STDIN_FILENO, F_GETFL);
    ERROR_EXIT(fcntl(STDIN_FILENO, F_SETFL, flags | O_ASYNC | O_NONBLOCK) == -1, "fcntl(F_SETFL)");
    ERROR_EXIT(tty_set_char_break(STDIN_FILENO, &ori) == -1, "tty_set_char_break");

    for (done = false, cnt = 0; !done; ++cnt) {
        for (int j = 0; j < 100000000; ++j) {
            continue;
        }
        if (got_sigio) {
            /* Signal-driven I/O provides edge-triggered notification. */
            while (read(STDIN_FILENO, &ch, 1) > 0 && !done) {
                printf("cnt=%d; read %c\n", cnt, ch);
                done = ch == '#';
            }
            got_sigio = 0;
        }
    }
    ERROR_EXIT(tcsetattr(STDIN_FILENO, TCSAFLUSH, &ori) == -1, "tcsetattr");
    return 0;
}

#endif