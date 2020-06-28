//
// Created by Steve on 4/24/2020.
//

#ifndef C4FUN_MACRO_H
#define C4FUN_MACRO_H

#include <stdio.h>

#define MAIN() \
int main()

#define MAIN_EX(argc, argv) \
int main(int argc, char *argv[])

#define WHERE_FALSE while(0)

#ifndef EXTENDED_ER
#define ERROR_RETURN(error, code) \
if(error){\
    return code;\
}
#else
#define ERROR_RETURN(error, code, block, fmt, ...) \
do{\
    if(error){\
        fprintf(stderr, fmt "\n", ##__VA_ARGS__);\
        block\
        return code;\
    }\
} WHERE_FALSE
#endif

#define SOCKADDR_EX(x) (struct sockaddr*)&x,sizeof x

#include <stdio.h>
#include <stdlib.h>

#define ERROR_EXIT(error, msg) \
if(error){\
    perror(msg);\
    exit(EXIT_FAILURE);\
}

#define log(fmt, ...) printf(fmt "\n",##__VA_ARGS__)
#define LOG log

#endif //C4FUN_MACRO_H
