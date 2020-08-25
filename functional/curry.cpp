//
// Created by Steve on 8/18/2020.
//

#include <tuple>

#include "is-detected.h"

namespace sun{
    template<class Op,typename...Args>
    using can_invoke_t=decltype(std::declval<Op>()(std::declval<Args>()...));

    template<typename Op,typename...Args>
    using can_invoke=sun::polyfill::is_detected<can_invoke_t,Op,Args...>;

    template<typename Op,typename...Args>
    struct curry_t{
        template<typename...Rest>
        constexpr decltype(auto) operator()(Rest&&...rest) const{
            curry_t<Op,Args...,Rest...> curry={op_,std::tuple_cat(args_,std::make_tuple(std::forward<Rest>(rest)...))};
            if constexpr(!can_invoke<Op,Args...,Rest...>::value){
                return curry;
            }else{
                return curry();
            }
        }

        constexpr decltype(auto) operator()() const{
            return std::apply(op_,args_);
        }

        Op op_;
        std::tuple<Args...> args_;
    };

    template<typename Op>
    constexpr curry_t<Op> curry(Op&&op){
        return {std::forward<Op>(op)};
    }
}

#include "macro.h"

int test(int x,int y,int z){
    return x+y+z;
}

MAIN(){
    auto f=sun::curry(test)(1);
    auto g=f(2);
    LOG("%d,%d",g(3),sun::curry(test)(1)(2,3));
}