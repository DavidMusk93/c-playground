//
// Created by Steve on 10/29/2020.
//

#include "macro.h"

MAIN(){
    char c=0xff;
    if(c==-1){
        LOG("YES,%d,%d",c,-1);
    }else{
        LOG("NO,%d,%d",c,-1);
    }
}