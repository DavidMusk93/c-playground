#include "macro.h"

#ifdef DEBUG
#define TEMPNODEBUG DEBUG
#undef DEBUG
#endif

#include "a.h"

#ifdef TEMPNODEBUG
#define DEBUG TEMPDEBUG
#undef TEMPNODEBUG
#endif

MAIN() {
    LOG("%d", a);
#ifdef DEBUG
    LOG("I'm debugger!");
#endif
}
