#ifndef MONITOR_PIPE_H
#define MONITOR_PIPE_H

#include "base.h"

namespace sun {
    class Pipe : public nocopy {
    public:
        Pipe();

        ~Pipe();

        int readEnd() const;

        int writeEnd() const;

        void closeReadEnd();

        void closeWriteEnd();

    public:
        static constexpr int kRead = 0;
        static constexpr int kWrite = 1;
    private:
        int fds_[2];
    };
}

#endif //MONITOR_PIPE_H
