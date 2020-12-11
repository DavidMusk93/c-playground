//
// Created by Steve on 10/12/2020.
//

#include <list>

#include "macro.h"

MAIN(){
    struct trivial{
        int*p;
    };
#define RETRIEVE_PTR(x) (*(void**)x)
    using NODE=std::pair<int,int>;
    using LIST=std::list<NODE>;
    LIST list;
    list.push_back({1,1});
    auto front=list.begin();
//    auto&p=*front;
    LOG("sizeof iterator %zu",sizeof(front));
    LOG("%p,%p,%p",&*list.begin(),RETRIEVE_PTR(&front),front._M_node);
    LIST::iterator it{nullptr};
    LOG("%p,%p",&it,RETRIEVE_PTR(&it));
//    int x=312;
//    trivial t{&x};
//    LOG("%p,%p,%p",&x,&t,RETRIEVE_PTR(&t));
}