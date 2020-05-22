//
// Created by Steve on 5/22/2020.
//

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <poll.h>

#include "macro.h"

int is_openable(const char *file) {
    pid_t pid;
    switch ((pid = fork())) {
        case -1: {
            perror("fork");
            return -1;
        }
        case 0: {
            alarm(1);
            int fd = open(file, O_RDWR);
            alarm(0);
            if (fd == -1) {
                perror("open");
                _exit(1);
            }
            _exit(0);
        }
        default: {
            int status;
            waitpid(pid, &status, 0);
            return status;
        }
    }
}

int is_writable(int fd) {
    struct pollfd pfd = {.fd=fd, .events=POLL_OUT};
    return poll(&pfd, 1, 1000);
}

#ifdef TEST_IS_OPENABLE
MAIN() {
#define TTY "/dev/ttyS0"
    if (is_openable(TTY) == 0) {
        int fd = open(TTY, O_RDWR);
        printf("Is writable: %d\n", is_writable(fd));
    }
    return 0;
}
#endif