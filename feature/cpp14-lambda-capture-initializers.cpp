//
// Created by Steve on 9/7/2020.
//

#include "macro.h"

MAIN(){
    auto generator=[x=0]()mutable{
        return x++;
    };
    auto x=1;
    auto f=[&r=x,x=x*10]{
        ++r;
        return r+x;
    };
    LOG("f()=%d",f());
    auto a=generator();
    auto b=generator();
    auto c=generator();
//    LOG("%d,%d,%d,%d",generator(),generator(),f(),x); /*eval sequence is undefined*/
    LOG("%d,%d,%d,%d",a,b,c,x);
}