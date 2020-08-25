//
// Created by Steve on 8/17/2020.
// @ref
//   *http://purecpp.org/detail?id=2188
//   *https://en.cppreference.com/w/cpp/types/void_t
//

#include <type_traits>

namespace sun{
    template<typename Fn,typename...Args>
    struct holder{};

    template<typename Holder,typename=void/*std::void_t<>*/>
    struct is_invocable_impl:std::false_type{};

    template<template<class Fn,class...Args> class Holder,typename Fn,typename...Args>
    struct is_invocable_impl<Holder<Fn,Args...>,std::void_t<decltype(std::declval<Fn>()(std::declval<Args>()...))>>:std::true_type{};

    template<typename Fn,typename...Args>
    using is_invocable=is_invocable_impl<holder<Fn,Args...>>;

    template<typename Fn,typename...Args>
    using is_invocable_t=typename is_invocable<Fn,Args...>::type;

    template<typename Fn,typename...Args>
    constexpr bool is_invocable_v=is_invocable<Fn,Args...>::value;
}

#include "macro.h"

void test1(int){}
void test2(){}

MAIN(){
    int a=1;
    static_assert(sun::is_invocable_v<decltype(test1),int>);
    static_assert(sun::is_invocable_v<decltype(test2)>);
    static_assert(!sun::is_invocable_v<decltype(a)>);
}