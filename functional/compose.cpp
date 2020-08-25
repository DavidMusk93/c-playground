//
// Created by Steve on 8/18/2020.
//

#include <functional>
#include "for-each.h"

template<class...Ops>
constexpr auto compose(Ops&&...ops){
    return [ops=std::make_tuple(std::forward<Ops>(ops)...)](auto&&...args){
        using R=decltype(std::get<sizeof...(Ops)-1>(ops)(std::forward<decltype(args)>(args)...));
        R res{};
        sun::for_each(ops,[&res,cap_args=std::make_tuple(std::forward<decltype(args)>(args)...)](auto&op,auto i){
            if constexpr(decltype(i)::value==sizeof...(Ops)-1){
//                res=std::invoke(op,args...); //can not capture varargs
                res=std::apply/*arguments as tuple*/(op,std::move(cap_args));
            }else{
                res=std::invoke(op,res);
            }
        });
        return res;
    };
}

#include "macro.h"

int f(int x){
    return x+1;
}

int g(int x){
    return x+2;
}

int h(int x){
    return x+3;
}

MAIN(){
    auto composite=compose(f,g,h);
    LOG("%d",composite(2));
}