//
// Created by Steve on 12/16/2020.
//

#include "macro.h"

MAIN(){
    u8 x=0B10'0000;
    LOG("%x",x|(-x));
    u32 a=24;
    LOG("%#x,%#x",a,-a);
}