//
// Created by Steve on 7/12/2020.
//

#include "macro.h"

MAIN() {
    int a, b;
    a = 1;
    b = 2;
    int &c = a;
    c = 3;
    c = b; /*single binding*/
    c = 4;
    LOG("%d,%d", a, b);
}