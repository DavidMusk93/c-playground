//
// Created by Steve on 11/19/2020.
//

#include "macro.h"

MAIN(){
    unsigned char a,b;
    a=8;
    b=-a;
    LOG("%x,%x,%x",a,b,a&b);
}