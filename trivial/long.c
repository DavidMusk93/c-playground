//
// Created by Steve on 8/31/2020.
//

#include "macro.h"

MAIN(){
#define u64 unsigned long
#define S64_MAX 0x7fffffffffffffffL
#define S64_MIN 0x8000000000000000L
    LOG("%ld,%ld,%ld",S64_MAX,S64_MIN,S64_MIN*-1);
}