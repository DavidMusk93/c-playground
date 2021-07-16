#include "macro.h"

#define SF(x) \
void x(){\
    LOG(#x);\
}

SF(a);
SF(b);

MAIN_EX(argc,argv){
    atoi(argv[1])>3?a():b();
}
