//
// Created by Steve on 9/7/2020.
//

#include <type_traits>

#include "macro.h"

auto f(const int&i){
    return i;
}

decltype(auto)/*keep references and cv-qualifiers*/ g(const int&i){
    return i;
}

MAIN(){
    int x=123;
    static_assert(/*std::is_same_v cpp17?*/std::is_same<const int&,decltype(f(x))>::value==0);
    static_assert(std::is_same<int,decltype(f(x))>::value==1);
    static_assert(std::is_same<const int&,decltype(g(x))>::value==1);
}