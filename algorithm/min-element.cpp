//
// Created by Steve on 7/23/2020.
//

#include <vector>
#include <algorithm>

using namespace std;

#include "macro.h"

MAIN(){
    vector<int> data{11,26,3,4,8,7,432,10};
    auto it=min_element(data.begin(),data.end());
    auto p=it.base();
    auto&n=*it;
    LOG("%p,%p,%p,%d,%d",&data[0],&*it,&n,int(it-data.begin()),*it);
}