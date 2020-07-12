//
// Created by Steve on 7/10/2020.
//

static inline int count_set_bits(int n) {
    unsigned int u = n;
    int count = 0;
    while (u) {
        count += (int) (u & 1U);
        u >>= 1U;
    }
    return count;
}

#include "macro.h"

MAIN() {
    int n = -1;
    LOG("builtin function: %d", __builtin_popcount(n));
    LOG("user function: %d", count_set_bits(n));
}