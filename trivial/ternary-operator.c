//
// Created by Steve on 7/15/2020.
// @ref
//   *http://arthurchiao.art/blog/system-call-definitive-guide-zh/
//

void *foo(void *v) {
    static int x = 312;
    return (v ?: &x);
}

#include "macro.h"

MAIN() {
    int x = 12;
    LOG("%d", *(int *) foo(&x));
    LOG("%d", *(int *) foo(0));
}