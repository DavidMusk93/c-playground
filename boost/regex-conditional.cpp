//
// Created by Steve on 11/21/2020.
//

#include <string>
#include <iostream>

#include <boost/regex.hpp>

#include "macro.h"

/**
 * @ref
 *   1) https://theboostcpplibraries.com/boost.regex
 *   2) https://www.regular-expressions.info/refadv.html
 */
MAIN(){
    std::string s="babxcac";
    boost::regex expr{"(a)?(?(1)b|c)"};
    boost::smatch what;
    if(boost::regex_search(s,what,expr)){
        for(auto&i:what){
            std::cout<<i<<std::endl;
        }
    }
}