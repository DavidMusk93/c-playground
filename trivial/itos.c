//
// Created by Steve on 6/4/2020.
//

#include "macro.h"

static const char *itos(int *iptr) {
    static __thread char buf[8];
    static char cs[] = "0123456789ABCDEF";
    unsigned char *p = (unsigned char *) &buf[8];
    unsigned char *in = (unsigned char *) iptr;
    int len = sizeof(int);
    while (len--) {
        *--p = cs[(*in) & 0xff];
        *--p = cs[(*in) >> 4];
        ++in;
    }
    return buf;
}

int main() {
    int i = 12;
    log("%s", itos(&i));
    return 0;
}