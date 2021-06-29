#include "macro.h"

#define USE_STATICMAPPING 1
//#define TEST_UTF8ENCODE 1
#ifdef TEST_UTF8ENCODE
#include <assert.h>
#endif

struct st_family_name_t {
    unsigned x;
    unsigned char a[sizeof(long)];
};

struct st_utf8_stream_t {
    const unsigned char *p;
    const unsigned char *e;
};

struct st_pair_t {
    unsigned x;
    char c;
};

int pair_cmp(const void *a, const void *b) {
    return *(int *) a - *(int *) b;
}

void utf8_stream_init(struct st_utf8_stream_t *self, const unsigned char *p, unsigned len) {
    self->p = p;
    self->e = p + len;
}

static inline int utf8_decode(const unsigned char *p, const unsigned char *e, unsigned *x);

unsigned utf8_stream_next(struct st_utf8_stream_t *self, int *utf8size) {
    int c;
    unsigned u;
    if (self->p < self->e) {
        c = utf8_decode(self->p, self->e, &u);
        if (!c) {
            goto err;
        }
        self->p += c;
        if (utf8size) {
            *utf8size = c;
        }
        return u;
    }
    err:
    return -1;
}

static inline int utf8_encode(unsigned char *p, const unsigned char *e, unsigned x) {
#define UTF8MASK1 0
#define UTF8MASK2 0xc0
#define UTF8MASK3 0xe0
#define UTF8MASK4 0xf0
#define SUFFIXMASK 0x80
    int i, c, n;
    c = 0;
    n = 32 - __builtin_clz(x);
    if (n < 8) {
        *p = x & 0xff;
#undef UTF8MASK1
    } else if (n < 12) {
        c = 1;
        *p = ((x >> 6) & 0xff) | UTF8MASK2;
#undef UTF8MASK2
    } else if (n < 17) {
        c = 2;
        *p = ((x >> 12) & 0xff) | UTF8MASK3;
#undef UTF8MASK3
    } else if (n < 22) {
        c = 3;
        *p = ((x >> 18) & 0xff) | UTF8MASK4;
#undef UTF8MASK4
    }
    if (p + c >= e) {
        goto err;
    }
    for (i = c - 1; i >= 0; i--) {
        *++p = ((x >> (i * 6)) & 0x3f) | SUFFIXMASK;
#undef SUFFIXMASK
    }
    return c + 1;
    err:
    return 0;
}

#ifdef TEST_UTF8ENCODE

void utf8_encode_test() {
    char a[] = "孙明强";
    char t[4];
    char *e = t + sizeof(t);
    unsigned u;
    int c1, c2;
    struct st_utf8_stream_t stream;
    utf8_stream_init(&stream, (unsigned char *) a, strlen(a));
    while ((u = utf8_stream_next(&stream, &c1)) != -1) {
        c2 = utf8_encode((unsigned char *) t, (unsigned char *) e, u);
        assert(c1 == c2);
        LOG_INFO("%.*s", c1, t);
    }
}

#endif

static inline int utf8_decode(const unsigned char *p, const unsigned char *e, unsigned *x) {
//#define UTF8MASK1 0x80
//#define UTF8MASK2 0xe0
//#define UTF8MASK3 0xf0
//#define UTF8MASK4 0xf8
//#define SUFFIXMASK 0xc0
//#define TESTMASK(x,m,o) !((((x)&m)^(m-1))>>o)
#define PREFIXTEST1(x) (((x)>>7)==0)
#define PREFIXTEST2(x) (((x)>>5)==0x6)
#define PREFIXTEST3(x) (((x)>>4)==0xe)
#define PREFIXTEST4(x) (((x)>>3)==0x1e)
#define SUFFIXTEST(x) (((x)>>6)==0x2)
    int i, c = 0;
    unsigned u = 0;
    if (PREFIXTEST1(*p)) {
#undef PREFIXTEST1
        u |= *p;
    } else if (PREFIXTEST2(*p)) {
#undef PREFIXTEST2
        c = 1;
        u |= *p & 0x1f;
    } else if (PREFIXTEST3(*p)) {
#undef PREFIXTEST3
        c = 2;
        u |= *p & 0xf;
    } else if (PREFIXTEST4(*p)) {
#undef PREFIXTEST4
        c = 3;
        u |= *p & 0x7;
    } else {
        goto err;
    }
    if (p + c >= e) {
        goto err;
    }
    for (i = 0; i < c; i++) {
        if (!SUFFIXTEST(*++p)) {
#undef SUFFIXTEST
            goto err;
        }
        u <<= 6;
        u |= *p & 0x3f;
    }
    *x = u;
    return c + 1;
    err:
    return 0;
}

struct st_family_name_t table[128]
#ifdef USE_STATICMAPPING
        = {
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0x987e, .a={0xe9, 0xa1, 0xbe,}},
                {.x=0xffffffff, .a={}},
                {.x=0x5c39, .a={0xe5, 0xb0, 0xb9,}},
                {.x=0x6c5f, .a={0xe6, 0xb1, 0x9f,}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0x949f, .a={0xe9, 0x92, 0x9f,}},
                {.x=0x7c73, .a={0xe7, 0xb1, 0xb3,}},
                {.x=0xffffffff, .a={}},
                {.x=0x4f0d, .a={0xe4, 0xbc, 0x8d,}},
                {.x=0x859b, .a={0xe8, 0x96, 0x9b,}},
                {.x=0x59da, .a={0xe5, 0xa7, 0x9a,}},
                {.x=0x8d75, .a={0xe8, 0xb5, 0xb5,}},
                {.x=0x94b1, .a={0xe9, 0x92, 0xb1,}},
                {.x=0x5b59, .a={0xe5, 0xad, 0x99,}},
                {.x=0x674e, .a={0xe6, 0x9d, 0x8e,}},
                {.x=0x5468, .a={0xe5, 0x91, 0xa8,}},
                {.x=0x5434, .a={0xe5, 0x90, 0xb4,}},
                {.x=0x90d1, .a={0xe9, 0x83, 0x91,}},
                {.x=0x738b, .a={0xe7, 0x8e, 0x8b,}},
                {.x=0x51af, .a={0xe5, 0x86, 0xaf,}},
                {.x=0x9648, .a={0xe9, 0x99, 0x88,}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0x8d1d, .a={0xe8, 0xb4, 0x9d,}},
                {.x=0xffffffff, .a={}},
                {.x=0x5b5f, .a={0xe5, 0xad, 0x9f,}},
                {.x=0xffffffff, .a={}},
                {.x=0x798f, .a={0xe7, 0xa6, 0x8f,}},
                {.x=0x6c34, .a={0xe6, 0xb0, 0xb4,}},
                {.x=0x7aa6, .a={0xe7, 0xaa, 0xa6,}},
                {.x=0x7ae0, .a={0xe7, 0xab, 0xa0,}},
                {.x=0x4e91, .a={0xe4, 0xba, 0x91,}},
                {.x=0x82cf, .a={0xe8, 0x8b, 0x8f,}},
                {.x=0x6f58, .a={0xe6, 0xbd, 0x98,}},
                {.x=0x845b, .a={0xe8, 0x91, 0x9b,}},
                {.x=0x595a, .a={0xe5, 0xa5, 0x9a,}},
                {.x=0x8303, .a={0xe8, 0x8c, 0x83,}},
                {.x=0x5f6d, .a={0xe5, 0xbd, 0xad,}},
                {.x=0x90ce, .a={0xe9, 0x83, 0x8e,}},
                {.x=0x9c81, .a={0xe9, 0xb2, 0x81,}},
                {.x=0x97e6, .a={0xe9, 0x9f, 0xa6,}},
                {.x=0x660c, .a={0xe6, 0x98, 0x8c,}},
                {.x=0x9a6c, .a={0xe9, 0xa9, 0xac,}},
                {.x=0x82d7, .a={0xe8, 0x8b, 0x97,}},
                {.x=0x51e4, .a={0xe5, 0x87, 0xa4,}},
                {.x=0x82b1, .a={0xe8, 0x8a, 0xb1,}},
                {.x=0x65b9, .a={0xe6, 0x96, 0xb9,}},
                {.x=0x4fde, .a={0xe4, 0xbf, 0x9e,}},
                {.x=0x4efb, .a={0xe4, 0xbb, 0xbb,}},
                {.x=0x8881, .a={0xe8, 0xa2, 0x81,}},
                {.x=0x67f3, .a={0xe6, 0x9f, 0xb3,}},
                {.x=0x5510, .a={0xe5, 0x94, 0x90,}},
                {.x=0x7f57, .a={0xe7, 0xbd, 0x97,}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0x4f59, .a={0xe4, 0xbd, 0x99,}},
                {.x=0xffffffff, .a={}},
                {.x=0x891a, .a={0xe8, 0xa4, 0x9a,}},
                {.x=0x536b, .a={0xe5, 0x8d, 0xab,}},
                {.x=0x848b, .a={0xe8, 0x92, 0x8b,}},
                {.x=0x6c88, .a={0xe6, 0xb2, 0x88,}},
                {.x=0x97e9, .a={0xe9, 0x9f, 0xa9,}},
                {.x=0x6768, .a={0xe6, 0x9d, 0xa8,}},
                {.x=0x6731, .a={0xe6, 0x9c, 0xb1,}},
                {.x=0x79e6, .a={0xe7, 0xa7, 0xa6,}},
                {.x=0x5c24, .a={0xe5, 0xb0, 0xa4,}},
                {.x=0x8bb8, .a={0xe8, 0xae, 0xb8,}},
                {.x=0x4f55, .a={0xe4, 0xbd, 0x95,}},
                {.x=0x5415, .a={0xe5, 0x90, 0x95,}},
                {.x=0x65bd, .a={0xe6, 0x96, 0xbd,}},
                {.x=0x5f20, .a={0xe5, 0xbc, 0xa0,}},
                {.x=0x5b54, .a={0xe5, 0xad, 0x94,}},
                {.x=0x66f9, .a={0xe6, 0x9b, 0xb9,}},
                {.x=0x4e25, .a={0xe4, 0xb8, 0xa5,}},
                {.x=0x534e, .a={0xe5, 0x8d, 0x8e,}},
                {.x=0x91d1, .a={0xe9, 0x87, 0x91,}},
                {.x=0x9b4f, .a={0xe9, 0xad, 0x8f,}},
                {.x=0x9676, .a={0xe9, 0x99, 0xb6,}},
                {.x=0x59dc, .a={0xe5, 0xa7, 0x9c,}},
                {.x=0x621a, .a={0xe6, 0x88, 0x9a,}},
                {.x=0x8c22, .a={0xe8, 0xb0, 0xa2,}},
                {.x=0x90b9, .a={0xe9, 0x82, 0xb9,}},
                {.x=0x55bb, .a={0xe5, 0x96, 0xbb,}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
                {.x=0xffffffff, .a={}},
        }
#endif
;
struct st_pair_t pairs[73]
#ifdef USE_STATICMAPPING
        = {
                {.x=0x4e25, .c='q'},
                {.x=0x4e91, .c='E'},
                {.x=0x4efb, .c='V'},
                {.x=0x4f0d, .c='-'},
                {.x=0x4f55, .c='k'},
                {.x=0x4f59, .c='_'},
                {.x=0x4fde, .c='U'},
                {.x=0x51af, .c='8'},
                {.x=0x51e4, .c='R'},
                {.x=0x534e, .c='r'},
                {.x=0x536b, .c='b'},
                {.x=0x5415, .c='l'},
                {.x=0x5434, .c='5'},
                {.x=0x5468, .c='4'},
                {.x=0x5510, .c='Y'},
                {.x=0x55bb, .c='z'},
                {.x=0x595a, .c='I'},
                {.x=0x59da, .c='/'},
                {.x=0x59dc, .c='v'},
                {.x=0x5b54, .c='o'},
                {.x=0x5b59, .c='2'},
                {.x=0x5b5f, .c='?'},
                {.x=0x5c24, .c='i'},
                {.x=0x5c39, .c='%'},
                {.x=0x5f20, .c='n'},
                {.x=0x5f6d, .c='K'},
                {.x=0x621a, .c='w'},
                {.x=0x65b9, .c='T'},
                {.x=0x65bd, .c='m'},
                {.x=0x660c, .c='O'},
                {.x=0x66f9, .c='p'},
                {.x=0x6731, .c='g'},
                {.x=0x674e, .c='3'},
                {.x=0x6768, .c='f'},
                {.x=0x67f3, .c='X'},
                {.x=0x6c34, .c='B'},
                {.x=0x6c5f, .c='&'},
                {.x=0x6c88, .c='d'},
                {.x=0x6f58, .c='G'},
                {.x=0x738b, .c='7'},
                {.x=0x798f, .c='A'},
                {.x=0x79e6, .c='h'},
                {.x=0x7aa6, .c='C'},
                {.x=0x7ae0, .c='D'},
                {.x=0x7c73, .c='+'},
                {.x=0x7f57, .c='Z'},
                {.x=0x82b1, .c='S'},
                {.x=0x82cf, .c='F'},
                {.x=0x82d7, .c='Q'},
                {.x=0x8303, .c='J'},
                {.x=0x845b, .c='H'},
                {.x=0x848b, .c='c'},
                {.x=0x859b, .c='.'},
                {.x=0x8881, .c='W'},
                {.x=0x891a, .c='a'},
                {.x=0x8bb8, .c='j'},
                {.x=0x8c22, .c='x'},
                {.x=0x8d1d, .c='='},
                {.x=0x8d75, .c='0'},
                {.x=0x90b9, .c='y'},
                {.x=0x90ce, .c='L'},
                {.x=0x90d1, .c='6'},
                {.x=0x91d1, .c='s'},
                {.x=0x949f, .c='*'},
                {.x=0x94b1, .c='1'},
                {.x=0x9648, .c='9'},
                {.x=0x9676, .c='u'},
                {.x=0x97e6, .c='N'},
                {.x=0x97e9, .c='e'},
                {.x=0x987e, .c='#'},
                {.x=0x9a6c, .c='P'},
                {.x=0x9b4f, .c='t'},
                {.x=0x9c81, .c='M'},
        }
#endif
;
#ifndef USE_STATICMAPPING

const char *family_name_dump(const void *p) {
    static char a[32];
    char *q = &a[0];
    unsigned char *t;
    struct st_family_name_t *self = (struct st_family_name_t *) p;
    q += sprintf(q, "{.x=%#x,.a={", self->x);
    for (t = self->a; *t; t++) {
        q += sprintf(q, "%#x,", *t);
    }
    sprintf(q, "}}");
    return &a[0];
}

const char *pair_dump(const void *p) {
    static char a[32];
    struct st_pair_t *self = (struct st_pair_t *) p;
    sprintf(a, "{.x=%#x,.c='%c'}", self->x, self->c);
    return &a[0];
}

void dump_list(const void *a, size_t len, size_t itemsize, size_t itemsperline, const char *(*dump_fn)(const void *)) {
    char buf[4096];
    char *p = &buf[0];
    char *e = p + sizeof(buf);
    size_t i;
    int n;
    for (i = 0; i < len; i++, a = (char *) a + itemsize) {
        n = snprintf(p, e - p, "%s,", dump_fn(a));
        if (p + n >= e) {
            break;
        }
        p += n;
        if ((i + 1) % itemsperline == 0) {
            *p++ = '\n';
        }
        if (p == e) {
            p[-1] = 0;
            break;
        }
    }
    if (p != e) {
        *p = 0;
    }
    LOG_INFO("%s", buf);
}

__ATTR (constructor) void initialize() {
    char a[] = "赵钱孙李"
               "周吴郑王"
               "冯陈褚卫"
               "蒋沈韩杨"
               "朱秦尤许"
               "何吕施张"
               "孔曹严华"
               "金魏陶姜"
               "戚谢邹喻"
               "福水窦章"
               "云苏潘葛"
               "奚范彭郎"
               "鲁韦昌马"
               "苗凤花方"
               "俞任袁柳"
               "唐罗薛伍"
               "余米贝姚"
               "孟顾尹江"
               "钟";
    char b[] = "0123"
               "4567"
               "89ab"
               "cdef"
               "ghij"
               "klmn"
               "opqr"
               "stuv"
               "wxyz"
               "ABCD"
               "EFGH"
               "IJKL"
               "MNOP"
               "QRST"
               "UVWX"
               "YZ.-"
               "_+=/"
               "?#%&"
               "*";
    struct st_utf8_stream_t stream;
    struct st_family_name_t *p;
    int i, n, t, c;
    utf8_stream_init(&stream, (const unsigned char *) a, dimension_of(a) - 1);
    for (i = 0, n = dimension_of(table); i < n; i++) {
        table[i] = (struct st_family_name_t) {-1};
    }
    for (i = 0, n = dimension_of(b) - 1; i < n; i++) {
        p = &table[b[i]];
        t = utf8_stream_next(&stream, &c);
        if (t == -1) {
            break;
        }
        snprintf((char *) p->a, c + 1, "%s", (const char *) &stream.p[-c]);
        p->x = t;
        pairs[i] = (struct st_pair_t) {.x=t, .c=b[i]};
    }
    qsort(pairs, dimension_of(pairs), sizeof(struct st_pair_t), pair_cmp);
    dump_list(table, dimension_of(table), sizeof(*table), 4, family_name_dump);
    dump_list(pairs, dimension_of(pairs), sizeof(*pairs), 4, pair_dump);
}

#endif

MAIN_EX(argc, argv) {
#ifdef TEST_UTF8ENCODE
    utf8_encode_test();
    return 0;
#endif
    if (argc < 2) {
        LOG_ERROR("usage:%s name-list", argv[0]);
        return 1;
    }
    struct st_utf8_stream_t stream;
    char a[512];
    char *p = &a[0];
    unsigned t;
    struct st_pair_t *x, y;
    utf8_stream_init(&stream, (const unsigned char *) argv[1], strlen(argv[1]));
    for (;;) {
        t = utf8_stream_next(&stream, 0);
        if (t == -1) {
            break;
        }
        y.x = t;
        x = (struct st_pair_t *) bsearch(&y, pairs, dimension_of(pairs), sizeof(struct st_pair_t), pair_cmp);
        if (!x) {
            goto err;
        }
        *p++ = x->c;
    }
    *p = 0;
#define MAGNETPREFIX "magnet:?xt=urn:btih:"
    LOG_INFO(MAGNETPREFIX"%s", &a[0]);
    return 0;
    err:
    LOG_ERROR("invalid input");
    return 2;
}