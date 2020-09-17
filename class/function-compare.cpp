//
// Created by Steve on 9/11/2020.
//

#include "macro.h"

#include <functional>

MAIN(){
    auto a=[](int a){return a*a;};
    auto b=a;
    LOG("%d",a==b);
}