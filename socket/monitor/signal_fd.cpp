#include "signal_fd.h"

#include <sys/signalfd.h>

namespace sun {
    SignalFd::SignalFd(int sig) {
        sigemptyset(&mask_);
        sigaddset(&mask_, sig);
        fd_ = signalfd(-1, &mask_, SFD_NONBLOCK | SFD_CLOEXEC);
        if (fd_ != -1) {
            cleanup_ = Defer([this] { util::Close(fd_); });
            sigprocmask(SIG_BLOCK, &mask_, nullptr);
        }
        initialize();
    }
}