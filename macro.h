//
// Created by Steve on 4/24/2020.
//

#ifndef C4FUN_MACRO_H
#define C4FUN_MACRO_H

#include <stdio.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <stdlib.h>

#define MAIN() \
int main()

#define MAIN_EX(argc, argv) \
int main(int argc, const char *argv[])

#define ERROR_S strerror(errno)

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

#define dimension_of(x) (sizeof(x)/sizeof(*x))

#define NL '\n'

#ifdef __cplusplus
#define __NULL nullptr
#else
#define __NULL NULL
#endif

#define REVOKE_OUTPUT_BUFFER() \
setbuf(stdout,__NULL);\
setbuf(stderr,__NULL)

#define SCOPED_GUARD(fn) __attribute__((__cleanup__(fn)))

typedef char s8;
typedef short s16;
typedef int s32;
typedef long s64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;

#endif //C4FUN_MACRO_H
