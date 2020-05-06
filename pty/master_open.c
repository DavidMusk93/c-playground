//
// Created by Steve on 4/25/2020.
//

#define _XOPEN_SOURCE 600

#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "master_open.h"
#include "macro.h"


int pty_master_open(char *slave_name, size_t sn_len) {
    int mfd, saved_errno;
    char *p;
    mfd = posix_openpt(O_RDWR | O_NOCTTY);
    ERROR_RETURN(mfd == -1, -1);
    FD_OP_CHECK(grantpt(mfd) == -1, mfd, saved_errno, -1);
    FD_OP_CHECK(unlockpt(mfd) == -1, mfd, saved_errno, -1);
    FD_OP_CHECK(!(p = ptsname(mfd)), mfd, saved_errno, -1);
    if (strlen(p) < sn_len) {
        strncpy(slave_name, p, sn_len);
    } else {
        close(mfd);
        errno = EOVERFLOW;
        return -1;
    }
    return mfd;
}