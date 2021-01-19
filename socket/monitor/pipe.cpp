//
// Created by esgyn on 1/15/2021.
//

#include "pipe.h"

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

namespace sun {
    Pipe::Pipe() : fds_{-1, -1} {
        signal(SIGPIPE, SIG_IGN);
        pipe2(fds_, O_NONBLOCK | O_CLOEXEC);
    }

    Pipe::~Pipe() {
        closeReadEnd();
        closeWriteEnd();
    }

    int Pipe::readEnd() const {
        return fds_[kRead];
    }

    int Pipe::writeEnd() const {
        return fds_[kWrite];
    }

#define IMPLCLOSE(x) \
void Pipe::close##x##End(){\
    auto&fd=fds_[k##x];\
    if(fd!=-1){\
        close(fd);\
        fd=-1;\
    }\
}

    IMPLCLOSE(Read);

    IMPLCLOSE(Write);
#undef IMPLCLOSE
}