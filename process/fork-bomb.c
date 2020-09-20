//
// Created by Steve on 9/17/2020.
//

#include <unistd.h>

#include "macro.h"

MAIN(){
    if(fork()==0){
        LOG("I am child");
        exit(0);
    }
    LOG("I am parent");
    return 0;
}