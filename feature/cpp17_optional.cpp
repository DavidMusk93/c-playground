#include <optional>
#include <exception>

#include "macro.h"

MAIN() {
    std::optional<int> opt;
    LOG_INFO(opt.value_or(3));
    try {
        LOG_INFO(opt.value());
    } catch (std::exception &e) {
        LOG_INFO(e.what());
    }
    opt = 4;
    LOG_INFO(opt.value());
}