//
// Created by Steve on 8/18/2020.
//

#ifndef C4FUN_NONESUCH_H
#define C4FUN_NONESUCH_H

#include <type_traits>

namespace sun{
    namespace polyfill{
        struct nonesuch{
            nonesuch()=delete;
            ~nonesuch()=delete;
            nonesuch(const nonesuch&)=delete;
            nonesuch&operator=(const nonesuch&)=delete;
        };

        template<class Default,class AlwaysVoid,template<class...> class Op,class...Args>
        struct detector{
            using value_t=std::false_type;
            using type=Default;
        };

        template<class Default,template<class...> class Op,class...Args>
        struct detector<Default,std::void_t<Op<Args...>>,Op,Args...>{
            using value_t=std::true_type;
            using type=Op<Args...>;
        };

        template<template<class...> class Op,class...Args>
        using is_detected=typename detector<nonesuch,void,Op,Args...>::value_t;
    }
}

#endif //C4FUN_NONESUCH_H
