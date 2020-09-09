//
// Created by Steve on 9/7/2020.
// @ref https://github.com/AnthonyCalandra/modern-cpp-features#user-defined-literals
//

#include <stdlib.h>
#include <math.h>

long long operator ""_celsius(unsigned long long/*required*/ degree){
    return std::llround(degree*1.8+32);
}

int operator ""_int(const char*str,std::size_t/*both are required*/){
    return std::atoi(str);
}

#include "macro.h"

MAIN(){
    LOG("%lld,%d",24_celsius,"123"_int);
}