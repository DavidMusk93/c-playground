#ifndef MONITOR_WORKER_H
#define MONITOR_WORKER_H

#include <memory>

#include "loop.h"

namespace sun {
    class Worker : public Loop {
    public:
        enum class State {
            STARTUP,
            BUSY,
            IDLE,
            FINISH,
        };

        Worker();

        ~Worker() = default;

        State &state() {
            return state_;
        }

    private:
        Defer cleanup_;
        State state_;
    };
}

#endif //MONITOR_WORKER_H
