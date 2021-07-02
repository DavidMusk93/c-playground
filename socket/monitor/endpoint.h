#ifndef MONITOR_ENDPOINT_H
#define MONITOR_ENDPOINT_H

#include "base.h"

namespace sun {
    class EndPoint : public nocopy, public transferable {
    public:
        enum class State : char {
            NOTHINGNESS,
            INITIALIZED,
            TRANSFERRED,
        };

        EndPoint() : state_(State::NOTHINGNESS) {}

        virtual ~EndPoint() = default;

        virtual int transferOwnership() {
            state_ = State::TRANSFERRED;
            cleanup_.cancel();
            return fd_;
        };

        void initialize() {
            state_ = State::INITIALIZED;
        }

        State state() const {
            return state_;
        }

        bool valid() const {
            return state_ == State::INITIALIZED;
        }

    protected:
        State state_;
        Defer cleanup_;
    };
}

#endif //MONITOR_ENDPOINT_H
