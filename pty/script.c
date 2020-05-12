//
// Created by Steve on 5/2/2020.
// gcc *.c ../tty/utility.c ../time/now.c -I ../tty/ -I ..
//

#include "utility.h"
#include "macro.h"
#include "fork.h"
#include "../time/now.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <assert.h>

#define BUF_SIZE 256

struct termios ori;

static void child_handler(int sig) {
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG);
    if (pid == -1) {
        perror("waitpid");
        return;
    }
    fprintf(stdout, "%s script end\r\n", DEFAULT_TIME_FORMAT(now(0)));
}

static void sigwinch_handler(int sig) {
    struct winsize ws;
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) != -1) {
        printf("Caught SINWINCH, new window size: "
               "%d row * %d columns\r\n", ws.ws_row, ws.ws_col);
    }
}

static void tty_reset(void) {
    ERROR_EXIT(tcsetattr(STDIN_FILENO, TCSANOW, &ori) == -1, "tcsetattr");
}

#define WRITE(fd, buf, buf_len)\
do{\
    if(write(fd,buf,buf_len)!=buf_len){\
        perror("partial/failed write (" #fd ")");\
        break;\
    }\
} while(0)

MAIN_EX(argc, argv) {
    char sn[MAX_SNAME];
    char *shell;
    int master_fd, script_fd;
    struct winsize ws;
    fd_set in_fds;
    char buf[BUF_SIZE];
    int num_read;
    pid_t child;
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = &child_handler;
    sigaction(SIGCHLD, &sa, 0); /* avoid zombie */

    /* set SIGWINCH handler */
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGWINCH);
    int sig_fd = signalfd(-1, &mask, SFD_CLOEXEC | SFD_NONBLOCK);
    ERROR_EXIT(sig_fd == -1, "signalfd");
    sigprocmask(SIG_BLOCK, &mask, 0);

    ERROR_EXIT(tcgetattr(STDIN_FILENO, &ori) == -1, "tcgetattr");
    ERROR_EXIT(ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) < 0, "ioctl(TIOCGWINSZ)");
    ERROR_EXIT((child = pty_fork(&master_fd, sn, MAX_SNAME, &ori, &ws)) == -1, "pty_fork");
    if (child == 0) {
        shell = getenv("SHELL");
        if (!shell || !*shell) {
            shell = "/bin/bash";
        }
        fprintf(stdout, "%s script start\n", DEFAULT_TIME_FORMAT(now(0)));
        /*signal(SIGWINCH, &sigwinch_handler);*/
        execlp(shell, shell, (char *) 0);
        ERROR_EXIT(1, "execlp");
    }
    script_fd = open(argc > 1 ? argv[1] : "typescript", O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0777);
    ERROR_EXIT(script_fd == -1, "open typescript");
    tty_set_raw(STDIN_FILENO, &ori);
    ERROR_EXIT(atexit(tty_reset) != 0, "atexit");
    for (;;) {
        FD_ZERO(&in_fds);
        FD_SET(STDIN_FILENO, &in_fds);
        FD_SET(master_fd, &in_fds);
        FD_SET(sig_fd, &in_fds);
        struct signalfd_siginfo si;
        int max_fd = master_fd > sig_fd ? master_fd : sig_fd;
        ERROR_EXIT(select(max_fd + 1, &in_fds, 0, 0, 0) == -1, "select");
        if (FD_ISSET(STDIN_FILENO, &in_fds)) {
            num_read = read(STDIN_FILENO, buf, BUF_SIZE);
            if (num_read <= 0) {
                exit(EXIT_SUCCESS);
            }
            WRITE(master_fd, buf, num_read);
        }
        if (FD_ISSET(master_fd, &in_fds)) {
            num_read = read(master_fd, buf, BUF_SIZE);
            if (num_read <= 0) { /* end of file */
                /*int status = 0;
                if (waitpid(child, &status, WNOHANG) == child) {
                    log("child returned: %#x", status);
                }*/
                exit(EXIT_SUCCESS);
            }
            WRITE(STDOUT_FILENO, buf, num_read);
            WRITE(script_fd, buf, num_read);
        }
        if (FD_ISSET(sig_fd, &in_fds)) {
            read(sig_fd, &si, sizeof si);
            assert(si.ssi_signo == SIGWINCH);
            sigwinch_handler(si.ssi_signo);
        }
    }
    return 0;
}