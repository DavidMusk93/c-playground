#ifndef MONITOR_TIME_H
#define MONITOR_TIME_H

#include <functional>

namespace sun {
    constexpr double kThousand = 1000;
    constexpr double kMillion = 1000000;

    namespace utility {
        using TimeFormatter = std::function<const char *(char buf[32], int seconds, int milliseconds)>;

        double Milliseconds();

        bool ValidProcess(int pid);

        const char *Now(char buf[32], TimeFormatter formatter = {});

        int GetPid();

        int Sleep(int ms);
    }
}

#endif //MONITOR_TIME_H
