#ifndef MONITOR_TIME_WHEEL_H
#define MONITOR_TIME_WHEEL_H

#include "base.h"

#include <vector>
#include <memory>

namespace sun {
    template<typename T, size_t N>
    class TimeWheel : public nocopy {
    public:
        using Item = std::shared_ptr<T>;
        using Slot = std::vector<Item>;

        TimeWheel() : i_(N - 1) {
            static_assert(N > 3 && N < 100, "invalid N for TimeWheel");
        };

        ~TimeWheel() = default;

        Slot tick() {
            Slot slot;
            if (i_ == 0) {
                i_ = N;
            }
            wheel_[--i_].swap(slot);
            return slot;
        }

        Slot &current() {
            return wheel_[i_];
        }

    private:
        Slot wheel_[N];
        int i_;
    };
}

#endif //MONITOR_TIME_WHEEL_H
