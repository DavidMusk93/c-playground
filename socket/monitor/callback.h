#ifndef MONITOR_CALLBACK_H
#define MONITOR_CALLBACK_H

#include "sock.h"

namespace sun {
    using Callback = io::Poll::Entry::Callback;

    void Echo(int fd);

    void OnTcpipAccept(int fd, void *handler, const Callback &callback);
}

#endif //MONITOR_CALLBACK_H
