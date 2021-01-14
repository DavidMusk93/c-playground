//
// Created by Steve on 1/13/2021.
//

#include "macro.h"

#define ADDR(level) __builtin_extract_return_addr(__builtin_return_address(level))

void fa(void *ptr) {
    LOG("@%s %p,%p", __func__, &fa, ptr);
}

void fb(void *ptr) {
    LOG("@%s %p,%p", __func__, ADDR(0), ADDR(1));
    LOG("@FRAME_ADDRESS %p,%p", __builtin_frame_address(0), __builtin_frame_address(1));
    fa(ptr);
}

void fc(void *ptr) {
    LOG("@%s %p,%p", __func__, &fb, &fc);
    fb(ptr);
}

MAIN() {
    int magic = 0x312;
    fc(&magic);
}