//
// Created by Steve on 6/25/2020.
//

#include "macro.h"

MAIN() {
#define FMT "%#010x,%#010x"
#define TWICE(x) (x),((unsigned int)((x)<0?-(x):(x)))
    unsigned int x = 0x80000000;
    LOG("INT_MIN: "FMT, TWICE((int) x));
    LOG("(INT_MIN+1): "FMT, TWICE((int) x + 1));
    LOG("-1: "FMT, TWICE(-1));
    LOG("INT_MAX(2): "FMT, TWICE(((int) x + 1) * -1));
    LOG("(INT_MIN-1): "FMT, TWICE(x - 1));
    return 0;
}