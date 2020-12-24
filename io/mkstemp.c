//
// Created by esgyn on 12/24/2020.
//

#include <unistd.h>

#include "macro.h"

static void close_fd(void *ptr) {
    int fd = *(int *) ptr;
    if (fd != -1) {
        close(fd);
        *(int *) ptr = -1;
    }
}

static void trivial_read(int fd) {
    char buf[64];
    lseek(fd, 0, SEEK_SET);
    int nr = read(fd, buf, sizeof(buf));
    if (nr > 0) {
        LOG("@%s %.*s", __func__, nr, buf);
    }
}

MAIN() {
    int fd SCOPED_GUARD(close_fd);
    fd = -1;
    char buf[] = "/tmp/test-XXXXXX";
    fd = mkstemp(buf);
    if (fd != -1) {
        unlink(buf);
        write(fd, buf, sizeof(buf));
//        fsync(fd);
        trivial_read(fd);
    }
    LOG("%d,%s", fd, buf);
//    sleep(600);
}
