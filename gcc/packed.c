//
// Created by Steve on 5/29/2020.
//

#include <stdio.h>
#include <stdint.h>

struct slave_data_t {
    uint64_t uid;
    char status;
    int block;
    short end;
} __attribute__((packed));

#define OFFSET(type, m) (size_t)(&((type *)0)->m)

int main() {
    printf("%ld\n", sizeof(struct slave_data_t));
    printf("%lu %lu %lu %lu\n",
           OFFSET(struct slave_data_t, uid),
           OFFSET(struct slave_data_t, status),
           OFFSET(struct slave_data_t, block),
           OFFSET(struct slave_data_t, end));
    return 0;
}