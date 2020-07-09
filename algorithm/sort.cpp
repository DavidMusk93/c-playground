//
// Created by Steve on 7/6/2020.
//

#include <vector>
#include <algorithm>

#include "macro.h"

MAIN() {
    std::vector<int> data{2, 4, 5, 1, 8, 7};
    std::sort(&data[2], &data[data.size()]);
    for (auto i:data) {
        LOG("%d", i);
    }
}