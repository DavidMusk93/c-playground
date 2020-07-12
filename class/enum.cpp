//
// Created by Steve on 7/6/2020.
//

#include <iostream>

enum A {
    A_1 = 1,
    A_2 = 2,
    A_BIG = 0xffffffffU,
};

enum B {
    B_1 = 1,
    B_2 = 2,
    B_BIG = 0xfffffffffUL,
};

#include "macro.h"

MAIN() {
    std::cout << sizeof(A_1) << std::endl; //4
    std::cout << A_BIG << std::endl; //4294967295
    std::cout << sizeof(B_1) << std::endl; //8
    std::cout << B_BIG << std::endl; //68719476735
}