#ifndef MONITOR_ENDPOINT_H
#define MONITOR_ENDPOINT_H

#include "base.h"

namespace sun {
    class EndPoint : public nocopy {
    public:
        virtual ~EndPoint() = default;

        explicit operator int() const {
            return fd_;
        };

        virtual int transferOwnership() {
            cleanup_.cancel();
            return fd_;
        };

    protected:
        int fd_{-1};
        Defer cleanup_;
    };
}

#endif //MONITOR_ENDPOINT_H
