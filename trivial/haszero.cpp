#include <limits.h>

#include "macro.h"

#define ONES ((size_t)-1/UCHAR_MAX)
#define HIGHS (ONES*(UCHAR_MAX/2+1))

MAIN(){
    LOG("%#018lx",ONES);
    LOG("%#018lx",HIGHS);
}