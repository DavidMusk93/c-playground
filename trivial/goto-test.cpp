//
// Created by Steve on 10/27/2020.
//

#include <vector>

#include "macro.h"

using namespace std;

MAIN(){
    int k=0;
retry:
    while(true){
        vector<int> v;
        for(int i=0;i<10000;++i){
            v.push_back(i);
        }
//        break;
        if(k++<5){
            goto retry;
        }else{
            break;
        }
    }
}