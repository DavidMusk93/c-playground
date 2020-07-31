//
// Created by Steve on 7/24/2020.
//

#include <algorithm>

#include "macro.h"

struct Pair{
    int h;
    int k;
};

MAIN(){
    Pair data[]={{5,0},{7,0},{5,2},{6,1},{4,4},{7,1}};
    std::sort(data,data+dimension_of(data),[](const auto&i,const auto&j){
        if(i.h<j.h){
            return true;
        }
        return i.h == j.h && i.k > j.k;
    });
    for(auto & i : data){
        LOG("%d,%d",i.h,i.k);
    }
}