#include "macro.h"

#ifdef _DEBUG // gcc -g would not import this flag
static int a=0;
#else
static int a = 1;
#endif

MAIN() {
    LOG("%d", a);
}