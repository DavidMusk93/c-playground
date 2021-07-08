#include "macro.h"

#include <functional>

MAIN() {
    std::function<void(void *)> f1;
    std::function<void(void)> f2 = [] {
        LOG_INFO("hi,there!");
    };
    LOG_INFO("%ld,%ld", sizeof(f1), sizeof(f2));
}