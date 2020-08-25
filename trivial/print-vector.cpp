//
// Created by Steve on 8/22/2020.
//

#include <iostream>
#include <vector>

#include "macro.h"

static std::ostream&operator<<(std::ostream&os,const std::vector<int>&v){
    for(auto&i:v){
        if(&i==&v[0]){
            os<<i;
        }else{
            os<<" "<<i;
        }
    }
    os<<std::endl;
    return os;
}

MAIN(){
    std::vector<int> data{1,2,3,4,5};
    std::cout<<data;
}