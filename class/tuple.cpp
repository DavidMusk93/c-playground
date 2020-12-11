//
// Created by Steve on 9/23/2020.
//

#include <type_traits>
#include <tuple>

#include "macro.h"

template<typename T>
std::enable_if_t<std::is_floating_point_v<T>,float> value(T&&t){
    t/=2;
    return t;
}

template<typename T>
std::enable_if_t<std::is_integral_v<T>,int> value(T&&t){
    t<<=1;
    return t;
}

template<typename...T/*,std::enable_if_t<(sizeof...(T)>1),void>* = nullptr*/>
decltype(auto) value(T&&...t){
    return std::make_tuple(value(std::forward<T>(t))...);
}

MAIN(){
    auto [f,i]=value(1.2,8);
    LOG("%f,%d",f,i);
}