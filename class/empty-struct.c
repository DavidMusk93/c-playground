//
// Created by Steve on 7/12/2020.
// @ref http://www.spongeliu.com/260.html
//

struct A_t {

};

struct B_t {
    int n;
};

#include "macro.h"

MAIN() {
    struct A_t array[2];
    struct A_t a;
    struct B_t b;
    LOG("empty array size: %d", (int) sizeof(array));
    LOG("%p,%p", &array[0], &array[1]); //same address
    LOG("%d", (int) sizeof(a));
    LOG("%d", (int) sizeof(b));
}