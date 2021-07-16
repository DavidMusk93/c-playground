#include "macro.h"

MAIN(){
    int a=0x7fffffff;
    int b=a+1;
    LOG("%#x,%#x",a,b);
}