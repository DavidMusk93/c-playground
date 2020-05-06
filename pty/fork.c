//
// Created by Steve on 4/25/2020.
//

#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "fork.h"
#include "master_open.h"
#include "macro.h"

pid_t pty_fork(int *mfd_ptr, char *slave_name, size_t sn_len, const struct termios *t, const struct winsize *ws) {
    int mfd, sfd, saved_errno;
    pid_t child;
    char sname[MAX_SNAME];
    ERROR_RETURN((mfd = pty_master_open(sname, MAX_SNAME)) == -1, -1);
    if (slave_name) {
        if (strlen(sname) < sn_len) {
            strncpy(slave_name, sname, sn_len);
        } else {
            close(mfd);
            errno = EOVERFLOW;
            return -1;
        }
    }
    FD_OP_CHECK((child = fork()) == -1, mfd, saved_errno, -1);
    /* Parent returned */
    if (child != 0) {
        *mfd_ptr = mfd;
        return child;
    }

    /* Child falls through to here */
    /* The child is the leader of the new session and loses its controlling terminal */
    ERROR_EXIT(setsid() == -1, "setsid");
    close(mfd);
    /* Since the child lost its controlling terminal in the previous step, this step
     * causes the pseudoterminal slave to become the controlling terminal for the child. */
    ERROR_EXIT((sfd = open(sname, O_RDWR)) == -1, "open slave");
#ifdef TIOCSCTTY /* Acquire controlling tty on BSD */
    /* Make the given terminal the controlling terminal of the calling process. The calling
     * process must be a session leader and not have a controlling terminal already. */
    ERROR_EXIT(ioctl(sfd, TIOCSCTTY, 0) == -1, "ioctl(TIOCSCTTY)");
#endif
    if (t) {
        ERROR_EXIT(tcsetattr(sfd, TCSANOW, t) == -1, "tcsetattr");
    }
    if (ws) {
        ERROR_EXIT(ioctl(sfd, TIOCSWINSZ, ws) == -1, "ioctl(TIOCSWINSZ)");
    }
    /* Duplicate pty slave to be child's stdin, stdout, and stderr */
    ERROR_EXIT(dup2(sfd, STDIN_FILENO) != STDIN_FILENO, "dup2(STDIN_FILENO)");
    ERROR_EXIT(dup2(sfd, STDOUT_FILENO) != STDOUT_FILENO, "dup2(STDOUT_FILENO)");
    ERROR_EXIT(dup2(sfd, STDERR_FILENO) != STDERR_FILENO, "dup2(STDERR_FILENO)");
    if (sfd > STDERR_FILENO) {
        close(sfd);
    }
    return 0;
}