//
// Created by Steve on 8/18/2020.
//

#ifndef C4FUN_FOR_EACH_H
#define C4FUN_FOR_EACH_H

#include <tuple>

namespace sun{
    template<typename...Args,typename Op,std::size_t...I>
    constexpr void for_each(const std::tuple<Args...>&args/*not generic*/,Op&&op,std::index_sequence<I...>){
        constexpr const size_t N=sizeof...(I);
        (std::forward<Op>(op)(std::get<N-I-1>(args),std::integral_constant<size_t,N-I-1>{}),/*fold expression op*/...); //(pack op...)
    }

    template<typename T,typename Op>
    constexpr void for_each(T&&t,Op&&op){
        constexpr const size_t SIZE=std::tuple_size_v<std::decay_t<T>>;
        for_each(std::forward<T>(t),std::forward<Op>(op),std::make_index_sequence<SIZE>{});
    }
}

#endif //C4FUN_FOR_EACH_H
