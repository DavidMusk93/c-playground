//
// Created by Steve on 7/8/2020.
//

#include <vector>

#include "macro.h"

MAIN() {
    std::vector<int *> empty{};
    auto p = empty.back();
    empty.pop_back();
    LOG("%p", p); //ERROR
}