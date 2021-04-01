//
// Created by Steve on 11/16/2020.
//

#ifndef C_PLAYGROUND_RAII_H
#define C_PLAYGROUND_RAII_H

#include <functional>

namespace sun {
    using Closure = std::function<void()>;

    class Defer {
    public:
        Defer() = default;

        explicit Defer(Closure closure) {
            closure_.swap(closure);
        }

        Defer(Defer &&defer) noexcept {
            *this = defer.move();
        }

        ~Defer() {
            if (closure_) {
                closure_();
            }
        }

        Defer &operator=(Defer &&defer) noexcept {
            closure_.swap(defer.closure_);
            return *this;
        }

        Defer &&move() {
            return static_cast<Defer &&>(*this);
        }

        void cancel() {
            Closure trivial;
            closure_.swap(trivial);
        }

        void trigger() {
            if (closure_) {
                Closure trivial;
                closure_();
                closure_.swap(trivial);
            }
        }

        void operator()() {
            trigger();
        }

        void swap(Defer &defer) {
            closure_.swap(defer.closure_);
        }

    private:
        Closure closure_;
    };
}

#endif //C_PLAYGROUND_RAII_H
