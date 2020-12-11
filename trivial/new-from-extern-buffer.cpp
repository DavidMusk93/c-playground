//
// Created by Steve on 11/19/2020.
//

#include "macro.h"

MAIN(){
    char a[8];
    int*x=new(a)int();
    *x=0x11223344;
    LOG("%#x,%#x,%#x,%#x",a[0],a[1],a[2],a[3]); //0x44,0x33,0x22,0x11
}