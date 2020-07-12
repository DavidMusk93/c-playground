//
// Created by Steve on 7/12/2020.
//

//c++ standard: no object shall have the dame address in memory as an other variable
//指针运算依赖sizeof(T)
struct A {

};

struct B {
    int n;
};

#include "macro.h"

MAIN() {
    B array[2];
    auto diff = &array[1] - &array[0]; //((char*)&array[1]-(char*)&array[0])/sizeof(B)
    LOG("%d", (int) sizeof(A));
    LOG("%d", (int) sizeof(B));
    LOG("diff: %d", (int) diff);
}