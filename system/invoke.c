#define USEMACROARGS

#include "macro.h"

#define ARGS a,b,c,d,e,f,g,h,i

int foo(ARGLIST(int, ARGS)) {
    LOG(ARGREPEAT(",", "%d", ARGS), ARGS);
    return ARGCONCAT(+, ARGS);
}

MAIN() {
    return foo(1, 2, 3, 4, 5, 6, 7, 8, 9);
}