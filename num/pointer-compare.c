//
// Created by Steve on 11/22/2020.
//

#include <assert.h>

#include "macro.h"

MAIN(){
    char buf[16];
    int*p=(int*)buf;
    char*q=(char*)buf+8;
    assert((void*)q>(void*)p);
}