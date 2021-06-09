#include "macro.h"

/*traf defines*/
typedef unsigned char BYTE;

struct desc {
    BYTE *data, *null;
    int type, charset, len, mode;
};

enum SQLTYPE_CODE {
    SQLTYPECODE_VARCHAR_WITH_LENGTH = -601,
};

/*help structs*/
//struct byte_reader {
//    unsigned char *data;
//    int len, off;
//    int ref;
//    read_fn read;
//};

struct byte_reader {
    BYTE *data, *end;
    int ref;

    int (*peak)(struct byte_reader *r, void *dest, int n);

    int (*read)(struct byte_reader *r, void *dest, int n);
};

static int byte_reader_peak(struct byte_reader *r, void *dest, int n) {
    if (r->data + n > r->end) {
        return -1;
    }
    memcpy(dest, r->data, n);
    return 0;
}

static int byte_reader_read(struct byte_reader *r, void *dest, int n) {
    if (r->peak(r, dest, n) == 0) {
        r->data += n;
        return 0;
    }
    return -1;
}

struct byte_reader *byte_reader_new(unsigned char *data, int len) {
    struct byte_reader *r = malloc(sizeof(struct byte_reader));
    r->data = data;
    r->end = r->data + len;
    r->ref = 1;
    r->peak = &byte_reader_peak;
    r->read = &byte_reader_read;
    return r;
}

void byte_reader_ref(struct byte_reader *r) {
    __sync_add_and_fetch(&r->ref, 1);
}

void byte_reader_unref(struct byte_reader *r) {
    if (__sync_sub_and_fetch(&r->ref, 1) == 0) {
        free(r);
    }
}

/*Subject-verb-object*/
void copy_column_varchar_notnull(struct byte_reader *r, struct desc *desc) {
    if (desc->len > 0x7fff) {
        int l = 0;
        r->peak(r, &l, sizeof l); /*real length*/
        r->read(r, desc->data, l + 2);
    } else {
        short l = 0;
        r->peak(r, &l, sizeof l);
        r->read(r, desc->data, l + 2);
    }
}

void copy_column_varchar_nullable(struct byte_reader *r, struct desc *desc) {
    short null = 0;
    r->peak(r, &null, sizeof null); /*peek null value*/
    r->read(r, desc->null, 2); /*read null info*/
    if (null != -1) { /*not null*/
        copy_column_varchar_notnull(r, desc);
    }
}

void copy_column_varchar_general(struct byte_reader *r, struct desc *desc) {
    if (desc->null) {
        r->read(r, desc->null, 2);
    }
    int l = desc->len; /*declaration length*/
    if (desc->type == SQLTYPECODE_VARCHAR_WITH_LENGTH) {
        if (l > 0x7fff) {
            l = (l + 3) & ~3;
            l += 4;
        } else {
            l = (l + 1) & ~1;
            l += 2;
        }
    }
    r->read(r, desc->data, l);
}