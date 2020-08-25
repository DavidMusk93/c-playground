//
// Created by Steve on 8/18/2020.
//

#include <iostream>
#include <vector>
#include <type_traits>
#include <iterator>
#include <climits>
#include <cstdint>

template<typename...Args>
int sum(Args&&...args){
    return (args+...+(1*2)); //(pack op...op init)
}

template<typename...Args>
void printer(Args&&...args){
    (std::cout<<...<<args/*(init op...op pack)*/)<<'\n';
}

template<typename T,typename...Args>
void push_back_vec(std::vector<T>&v,Args&&...args){
    static_assert((std::is_constructible_v<T,Args>&&...));
    (v.push_back(std::forward<Args>(args)),/*op*/...); //(pack op...)
}

template<class T,std::size_t...N>
constexpr T bswap_impl(T i,std::index_sequence<N...>){
#define SHIFT(op,x,y) ((x)op(y))
#define SHIFT_LEFT(x,y) SHIFT(<<,x,y)
#define SHIFT_RIGHT(x,y) SHIFT(>>,x,y)
    return (SHIFT_LEFT(SHIFT_RIGHT(i,N*CHAR_WIDTH)&std::uint8_t(-1),(sizeof(T)-1-N)*CHAR_WIDTH)|/*op*/...); //(pack op...)
//    return (((i>>N*CHAR_WIDTH&std::uint8_t(-1))<<(sizeof(T)-1-N)*CHAR_WIDTH)|...);
#undef SHIFT
}

template<class T/*,class U=std::make_unsigned<T>*/>
constexpr decltype(auto) bswap(T i){
    return bswap_impl<T>(i,std::make_index_sequence<sizeof(T)>{});
}

#include "macro.h"

MAIN(){
    LOG("%d",sum(1,2,3));
    printer(1,2,3,"abc");
    std::vector<int> v;
    push_back_vec(v,1,2,3);
    std::copy(v.begin(),v.end(),std::ostream_iterator<int>(std::cout,","));
    LOG("");
    static_assert(bswap<std::uint16_t>(0x1234u)==0x3412u);
    static_assert(bswap<std::uint64_t>(0x0123456789abcdefULL)==0xefcdab8967452301ULL);
}