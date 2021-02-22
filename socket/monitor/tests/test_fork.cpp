#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <poll.h>

#include "test.h"
#include "util.h"
#include "pipe.h"

void redirect_output(const char *file) {
    int fd = open(file, O_RDWR | O_CREAT | O_APPEND | O_CLOEXEC);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    close(fd);
    setbuf(stdout, 0);
    setbuf(stderr, 0);
}

MAIN() {
#define PROCESSPREFIX "sun.test."
#define LOCKFILE "/tmp/" PROCESSPREFIX "lock"
#define LOGFILE "/tmp/" PROCESSPREFIX "log"
    redirect_output(LOGFILE);
    int fd = open(LOCKFILE, O_RDWR | O_CREAT | O_CLOEXEC);
    struct flock fl{};
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fcntl(fd, F_SETLK, &fl);
    sun::Pipe pipe;
    const char *exe = sun::util::WhichExe();
    int pid = fork();
    if (pid) {
        sun::util::NiceName(PROCESSPREFIX "main");
        pipe.closeReadEnd();
        waitpid(pid, 0, 0);
        LOGINFO("main start");
        sleep(60);
        LOGINFO("main end");
    } else if (fork()) {
        exit(0);
    } else {
        setsid();
        sun::util::NiceName(PROCESSPREFIX "daemon");
        pipe.closeWriteEnd();
        do {
            int rval;
            struct pollfd pfd{.fd=pipe.readEnd(), .events=POLLIN};
            POLL(rval, poll, &pfd, 1, -1);
        } while (0);
        LOGINFO("master dead,revoke it");
        execl(exe, exe, 0);
    }
}