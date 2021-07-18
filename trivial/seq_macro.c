#include "macro.h"

void foo(int i) {
    LOGINFO("%d", i);
}

MAIN() {
    LOGINFO("%d,%d", _BOOL(0), _BOOL(123));
    _IF_ELSE(12)(LOGINFO("yes");)(LOGINFO("no"))
#define FOO(x) foo(x),
    EVAL(MAP(FOO, 1, 2, 3, 4)) 0;
}