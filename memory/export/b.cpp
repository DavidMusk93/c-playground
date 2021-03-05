#include "a.h"

#include "macro.h"

MAIN() {
    struct {
        A a;
        void *p{};
    } wrapper;
#define a wrapper.a
    LOG("sizeof A:%d", (int) sizeof(A));
    LOG("address of a:%p", &a);
    a.dump();
#undef a
}