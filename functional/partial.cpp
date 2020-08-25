//
// Created by Steve on 8/18/2020.
//

#include <tuple>


namespace sun{
    template<typename Op,typename...Args>
    class partial_t{
    public:
        constexpr partial_t(Op&&op,Args&&...args):op_(std::forward<Op>(op)),args_(std::forward_as_tuple(args...)){}

        template<typename...Rest>
        constexpr decltype(auto) operator()(Rest&&...rest){
            return std::apply(op_,std::tuple_cat(args_,std::forward_as_tuple(std::forward<Rest>(rest)...)));
        }

    private:
        Op op_;
        std::tuple<Args...> args_;
    };

    template<typename Op,typename...Args>
    constexpr decltype(auto) partial(Op&&op,Args&&...args){
        return partial_t<Op,Args...>(std::forward<Op>(op),std::forward<Args>(args)...);
    }
}

#include "macro.h"

int test(int x,int y,int z){
    return x+y+z;
}

MAIN(){
    auto fn=sun::partial(test,5,3);
    LOG("%d",fn(7));
    LOG("%d,%d",sun::partial(test)(5,3,7),sun::partial(test,5)(3,7));
}