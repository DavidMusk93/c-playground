//
// Created by Steve on 4/25/2020.
//

#ifndef C4FUN_FORK_H
#define C4FUN_FORK_H

#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#define MAX_SNAME 1024

pid_t pty_fork(int *mfd_ptr, char *slave_name, size_t sn_len, const struct termios *t, const struct winsize *ws);

#endif //C4FUN_FORK_H
