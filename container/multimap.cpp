//
// Created by Steve on 8/4/2020.
//

#include <unordered_map>

#include "macro.h"

MAIN(){
    std::unordered_multimap<int,int> um;
    um.insert({1,2});
    um.insert({1,3});
    um.insert({2,3});
    auto range=um.equal_range(1);
    LOG("count: %d",(int)um.count(1));
    for(auto it=range.first;it!=range.second;++it){
        LOG("%d,%d",it->first,it->second);
    }
    um.erase(1);
    LOG("***");
    for(auto&p:um){
        LOG("%d,%d",p.first,p.second);
    }
}