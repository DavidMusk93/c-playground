#ifndef MONITOR_SIGNAL_FD_H
#define MONITOR_SIGNAL_FD_H

#include "endpoint.h"

#include <sys/signal.h>

namespace sun {
    class SignalFd : public EndPoint {
    public:
        SignalFd(int sig);

    private:
        sigset_t mask_;
    };
}

#endif //MONITOR_SIGNAL_FD_H
