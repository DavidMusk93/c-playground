//#define TEST 1
#ifdef TEST
#define USEMACROARGS
#endif

#include "macro.h"

static char base64_table[256];
static char base64_rtable[256];

static GCCATTRCTOR void base64_table_init() {
#define ASSIGN(__x, __y) \
base64_table[__x]=__y;\
base64_rtable[__y]=__x
    int i;
    for (i = 0; i < 26; ++i) {
        ASSIGN(i, 'A' + i);
        ASSIGN(i + 26, 'a' + i);
    }
    for (i = 0; i < 10; i++) {
        ASSIGN(i + 52, '0' + i);
    }
    ASSIGN(62, '+');
    ASSIGN(63, '/');
#undef ASSIGN
    base64_rtable['='] = 0; /*padding character*/
}

#define BASE64GROUPBYTECOMMON(__prefix, __type, __x, __self) static inline unsigned char __prefix##__x(__type*self)
#ifdef TEST
union base64_group_u_BAD {
    int i;
    char a[4];
    struct {
        unsigned char _0h: 6;
        unsigned char _0l: 2;
        unsigned char _1h: 4;
        unsigned char _1l: 4;
        unsigned char _2h: 2;
        unsigned char _2l: 6;
        unsigned char _3;
    } b;
};

void base64_group_u_BAD_print(union base64_group_u_BAD *self) {
#define ARGS _0h,_0l,_1h,_1l,_2h,_2l,_3
    LOG(ARGREPEAT(",", "%#x", ARGS), ARGLIST(self->b., ARGS));
#undef ARGS
}

union base64_group_u_RIGHT { /*consider endian*/
    int i; /*fast reset*/
    char a[4]; /*easy copy*/
    struct { /*usage*/
#define FIELD(__name, __bits) unsigned char __name:__bits
        FIELD(_0l, 2);
        FIELD(_0h, 6);
        FIELD(_1l, 4);
        FIELD(_1h, 4);
        FIELD(_2l, 6);
        FIELD(_2h, 2);
#undef FIELD
    } b;
};
#define BASE64GROUPBYTERIGHT(__x, __self) BASE64GROUPBYTECOMMON(base64_group_RIGHT_b,union base64_group_u_RIGHT,__x,__self)

BASE64GROUPBYTERIGHT(0, self) {
    return self->b._0h;
}

BASE64GROUPBYTERIGHT(1, self) {
    return (self->b._0l << 4) + self->b._1h;
}

BASE64GROUPBYTERIGHT(2, self) {
    return (self->b._1l << 2) + self->b._2h;
}

BASE64GROUPBYTERIGHT(3, self) {
    return self->b._2l;
}

#endif

union base64_group_u {
    int i;
    char a[4];
    unsigned char b[4];
};
#define BASE64GROUPBYTE(__x, __self) BASE64GROUPBYTECOMMON(base64_group_b,union base64_group_u,__x,__self)

BASE64GROUPBYTE(0, self) {
    return self->b[0] >> 2;
}

BASE64GROUPBYTE(1, self) {
    return (((self->b[0]) & 0x3) << 4) + (self->b[1] >> 4);
}

BASE64GROUPBYTE(2, self) {
    return (((self->b[1]) & 0xf) << 2) + (self->b[2] >> 6);
}

BASE64GROUPBYTE(3, self) {
    return self->b[2] & 0x3f;
}

#undef BASE64GROUPBYTECOMMON
#define INPUTSTRCHECK(__x, __error_code) if(!__x||!*__x)return __error_code

char *base64_decode(const char *src) {
    INPUTSTRCHECK(src, 0);
    union base64_group_u group;
    int sz = (int) strlen(src);
    if (sz & 0x3) { /*length checking*/
        return 0;
    }
    char *dst = malloc(sz / 4 * 3 + 1/*null-terminated*/);
    if (dst) {
        char *p = dst;
        unsigned char *q = (unsigned char *) src;
        for (; *q; p += 3, q += 4) {
            group.i = 0;
            unsigned char x = base64_rtable[q[0]];
            group.b[0] |= x << 2; /*h6*/
            x = base64_rtable[q[1]];
            group.b[0] |= x >> 4; /*l2*/
            group.b[1] |= (x & 0xf) << 4; /*h4*/
            x = base64_rtable[q[2]];
            group.b[1] |= x >> 2; /*l4*/
            group.b[2] |= (x & 0x3) << 6; /*h2*/
            x = base64_rtable[q[3]];
            group.b[2] |= x; /*l6*/
            memcpy(p, group.a, 3);
        }
        *p = 0;
    }
    return dst;
}

char *base64_encode(const char *src) {
    int done = 0;
#ifdef TEST
    union base64_group_u_RIGHT group;
#else
    union base64_group_u group;
#endif
    char *dst = malloc((strlen(src) + 2/*module padding*/) / 3 * 4 + 1/*null-terminated*/);
    char *p = dst;
    while (!done) {
        int n;
        group.i = 0;/*initialize*/
        for (n = 0; n < 3; ++n) {
            if (!*src) {
                done = 1;
                break;
            }
            group.a[n] = *src++;
        }
        if (n > 0) {
#ifdef TEST
#define BASE64GROUPCHAR(x) base64_table[base64_group_RIGHT_b##x(&group)]
#else
#define BASE64GROUPCHAR(x) base64_table[base64_group_b##x(&group)]
#endif
            *p++ = BASE64GROUPCHAR(0);
            *p++ = BASE64GROUPCHAR(1);
            *p++ = n < 2 ? '=' : BASE64GROUPCHAR(2);
            *p++ = n < 3 ? '=' : BASE64GROUPCHAR(3);
#undef BASE64GROUPCHAR
        }
    }
    *p = 0;
    return dst;
}

static void free_pp(char **pp) {
    free(*pp);
}

MAIN_EX(argc, argv) {
#ifdef TEST
    union base64_group_u_BAD bad;
    bad.i = 0x11223344;
    base64_group_u_BAD_print(&bad);
#endif
    union base64_group_u group;
    LOG("sizeof(union base64_group_u)=%d,%d", (int) sizeof(union base64_group_u), (int) sizeof(group.b));
    if (argc > 1) {
        char *s1 GCCATTRCLEANUP(free_pp) = base64_encode(argv[1]);
        char *s2 GCCATTRCLEANUP(free_pp) = base64_decode(s1);
        LOG("'%s','%s','%s'", argv[1], s1, s2);
    }
    return argc == 1;
}
