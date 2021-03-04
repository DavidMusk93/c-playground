#include "a.h"

#include "macro.h"

MAIN() {
    struct {
        A a;
        void *p{};
    } wrapper;
#define a wrapper.a
    LOG("address of a:%p", &a);
    a.dump();
#undef a
}