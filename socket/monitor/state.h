#ifndef MONITOR_STATE_H
#define MONITOR_STATE_H

#include <atomic>

class stateful {
public:
    enum class State : char {
        NOTHINGNESS,
        INITIALIZED,
        TRANSFERRED,
        IDLE,
        WORK,
        TERMINATED,
    };

    stateful() : state_(State::NOTHINGNESS) {}

    bool initialized() const { return state_.load(std::memory_order_relaxed) >= State::INITIALIZED; }

    State getstate() const { return state_.load(std::memory_order_acquire); }

    void setstate(State state) { state_.store(state, std::memory_order_release); }

private:
    std::atomic<State> state_;
};

#endif //MONITOR_STATE_H
