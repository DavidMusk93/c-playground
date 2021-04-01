#include "macro.h"

void *operator new(std::size_t size) {
    LOG("%s,%zu", __func__, size);
    return malloc(size);
}

MAIN() {
    int *a = new int(2);
    int *b = std::new int(3);
    LOG("%d,%d", *a, *b);
    delete a;
    delete b;
}