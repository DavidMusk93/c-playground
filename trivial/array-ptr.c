//
// Created by Steve on 1/13/2021.
//

#include <assert.h>

#include "macro.h"

typedef int(INTARRAY)[5];

#define ARRAY_FOREACH(a, i) for(int i=0,__n=sizeof(a)/sizeof(a[0]);i<__n;++i)

#define PTRDIFF(x, y) (int)((char*)(x)-(char*)(y))

MAIN() {
    int a[5];
    INTARRAY *b = &a, c;
    LOG("%p,%p", b, b + 1);
    LOG("%d,%d", PTRDIFF(a, &a), PTRDIFF(&a + 1, a));
    LOG("%d", (int) sizeof(c));
    ARRAY_FOREACH(c, i) {
        c[i] = 1 << i;
    }
}

//int main() {
//    int a[5];
//    assert(a == (int *) &a);
//    assert((char *) a - (char *) (&a - 1) == sizeof(a));
//}