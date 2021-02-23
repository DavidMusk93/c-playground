#ifndef MONITOR_NOCOPY_H
#define MONITOR_NOCOPY_H

namespace sun {
    class nocopy {
    public:
        nocopy(const nocopy &) = delete;

        nocopy &operator=(const nocopy &) = delete;

    protected:
        nocopy() = default;

        ~nocopy() = default;
    };
}

#endif //MONITOR_NOCOPY_H
