//
// Created by Steve on 11/22/2020.
//
#define USEMACROARGS

#include "macro.h"

#define OFFSET(type, name) (size_t)(&((type*)0)->name)

struct A {
    int *p;
    short x, y;
};

typedef struct S1 {
    char a;
    char b;
    double c;
} S1;

#pragma pack(push, 8)
typedef struct S2 {
    char c;
    double d;
    short s;
    char a[9];
    int i;
} S2;

typedef struct S3 {
    char c;
    S2 st;
    char a[10];
    int i;
} S3;

typedef struct S4_8 {
    int i;
    double d;
    char *p;
    char c;
    int *q;
} S4_8;
#pragma pack(pop)
#pragma pack(push, 4)
typedef struct S4_4 {
    int i;
    double d;
    char *p;
    char c;
    int *q;
} S4_4;
#pragma pack(pop)

MAIN() {
#define DUMPOFFSET(__t, ...) LOG(#__t ":%zu " ARGREPEAT(",","%zu",__VA_ARGS__),sizeof(__t),ARGLIST((size_t)&((__t*)0)->,__VA_ARGS__))
    DUMPOFFSET(struct A, p, x, y);
    DUMPOFFSET(S1, a, b, c);
    DUMPOFFSET(S2, c, d, s, a, i);
    DUMPOFFSET(S3, c, st, a, i);
    DUMPOFFSET(S4_8, i, d, p, c, q);
    DUMPOFFSET(S4_4, i, d, p, c, q);
}
