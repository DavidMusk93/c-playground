#include "macro.h"

// @ref https://mp.weixin.qq.com/s/WrH3i-pWmyDnH1HRK62s1A
int quorum_vote(int *a, int len) {
    int i, c, v;
    for (i = 0, c = 0; i < len; ++i) {
        if (c == 0) {
            v = a[i];
        }
        c += v == a[i] ? 1 : -1;
    }
    return v;
}

MAIN() {
    int a[] = {1, 2, 3, 2, 5, 2, 2};
    LOG("%d", quorum_vote(a, dimension_of(a)));
}