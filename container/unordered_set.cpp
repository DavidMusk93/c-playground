//
// Created by Steve on 7/5/2020.
//

#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "macro.h"

MAIN() {
    std::vector<int> data{100, 4, 200, 1, 3, 2, 4};
    std::unordered_set<int> us(data.begin(), data.end());
    std::unordered_map<int, int> um;
    for (auto i:data) {
        um[i] = 1;
    }
    for (auto &i:us) {
        LOG("%d", i);
    }
    auto k = um[20];
    ++um[1000];
    LOG("k: %d", k);
    for (auto &p:um) {
        LOG("%d<->%d", p.first, p.second);
    }
}