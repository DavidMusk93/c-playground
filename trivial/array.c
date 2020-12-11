//
// Created by Steve on 11/16/2020.
//

#include "macro.h"

MAIN(){
    int x[6]={}; /*array of integer*/
    int*a[6]={}; /*array of integer pointer*/
    int (*b)[6]=&x; /*pointer to array of integer*/
    LOG("%p,%p,%p",a,b,x);
}