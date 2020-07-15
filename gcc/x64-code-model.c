//
// Created by Steve on 7/15/2020.
// @ref
//   *https://eli.thegreenplace.net/2012/01/03/understanding-the-x64-code-models
//

#include <unistd.h>

#include "macro.h"

#define LITTLE_SIZE 100
#define BIG_SIZE 50000

int global_arr[LITTLE_SIZE] = {2, 3}; //visible by all other objects linked into the program
static int static_arr[LITTLE_SIZE] = {9, 7}; //visible only in this source file
int global_arr_big[BIG_SIZE] = {5, 6};
static int static_arr_big[BIG_SIZE] = {10, 20};

int global_func(int param) {
    return param * 10;
}

MAIN_EX(argc, argv) {
    char data[] = "david";
    int t = global_func(argc);
    t += global_arr[7];
    t += static_arr[7];
    t += global_arr_big[7];
    t += static_arr_big[7];
    write(1, data, sizeof(data));
    return t;
}