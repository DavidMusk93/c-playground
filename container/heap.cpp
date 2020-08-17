//
// Created by Steve on 7/31/2020.
//

#include <algorithm>
#include <functional>
#include <queue>
#include <vector>

#include "macro.h"

MAIN(){
//    auto i=std::less<>()(1,2);
//    auto j=std::greater<>()(3,2);
//    LOG("%d,%d",i,j);
    std::priority_queue<int> heap; //less (default) for maximal heap, greater for minimal
    std::vector<int> data{2,5,9,1,4,7,12,3};
    for(auto&i:data){
        heap.push(i);
    }
    while(!heap.empty()){
        auto top=heap.top();
        heap.pop();
        LOG("%d",top);
    }
}