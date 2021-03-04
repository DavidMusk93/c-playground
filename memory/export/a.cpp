#include "a.h"

#include "macro.h"

void A::dump() {
#ifdef DEBUG
    LOG("%d,%p", a, p);
#else
    LOG("%d", a);
#endif
}