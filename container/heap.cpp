//
// Created by Steve on 7/31/2020.
//

#include <algorithm>
#include <functional>

#include "macro.h"

MAIN(){
    auto i=std::less<>()(1,2);
    auto j=std::greater<>()(3,2);
    LOG("%d,%d",i,j);
}