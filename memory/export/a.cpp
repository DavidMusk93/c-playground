#include "a.h"

#include "macro.h"

void A::dump() {
    LOG("sizeof A:%d", (int) sizeof(A));
#ifdef DEBUG
    LOG("%d,%p", a, p);
#else
    LOG("%d", a);
#endif
}