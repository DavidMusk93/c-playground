//
// Created by Steve on 8/29/2020.
//

#include <vector>
#include <set>

#include "macro.h"

MAIN(){
    std::vector<int> data{4,3,1,6,8,5};
    std::set<int> set;
    for(auto i:data){
        set.insert(i);
    }
    for(auto i:set){
        LOG("%d",i);
    }
}