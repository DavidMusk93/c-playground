//
// Created by Steve on 6/8/2020.
//

#include "macro.h"

MAIN() {
#define HEX_FMT "%#010x\n"
    int n = 0x1001018;
    int m = -n;
    log(HEX_FMT HEX_FMT HEX_FMT HEX_FMT, n, m, (~(n - 1)), n & m);
    return 0;
}