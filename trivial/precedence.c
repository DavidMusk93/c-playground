//
// Created by Steve on 11/16/2020.
//

#include "macro.h"

MAIN(){
    int data[]={1,2,3,4,5,6};
    const int N=dimension_of(data);
    int*a[N+1];
    for(int i=0;i<N;++i){
        a[i]=data+i;
    }
    a[N]=0;
    int i=0;
    int target=3;
    while(a[i]&&*a[i]!=target){++i;}
//    while(a[i]){a[i]=a[++i];}
    for(;a[i];a[i]=a[i+1],++i);
    i=0;
    while(a[i]){
        LOG("%d,%d",i,*a[i]);
        ++i;
    }
}