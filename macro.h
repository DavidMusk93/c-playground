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
#include "raii.h"
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
typedef float f32;
typedef double f64;

#ifdef USEMACROARGS
#define _ARGS_17(_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,...) _16
#define ARGS_LEN(...) _ARGS_17(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)

#define _CONCAT(x,y) x##y
#define CONCAT(x,y) _CONCAT(x,y)

#define __ARGLIST_1(__t, __x) __t __x
#define __ARGLIST_2(__t, __x, ...) __ARGLIST_1(__t,__x),__ARGLIST_1(__t,__VA_ARGS__)
#define __ARGLIST_3(__t, __x, ...) __ARGLIST_1(__t,__x),__ARGLIST_2(__t,__VA_ARGS__)
#define __ARGLIST_4(__t, __x, ...) __ARGLIST_1(__t,__x),__ARGLIST_3(__t,__VA_ARGS__)
#define __ARGLIST_5(__t, __x, ...) __ARGLIST_1(__t,__x),__ARGLIST_4(__t,__VA_ARGS__)
#define __ARGLIST_6(__t, __x, ...) __ARGLIST_1(__t,__x),__ARGLIST_5(__t,__VA_ARGS__)
#define __ARGLIST_7(__t, __x, ...) __ARGLIST_1(__t,__x),__ARGLIST_6(__t,__VA_ARGS__)
#define __ARGLIST_8(__t, __x, ...) __ARGLIST_1(__t,__x),__ARGLIST_7(__t,__VA_ARGS__)
#define __ARGLIST_9(__t, __x, ...) __ARGLIST_1(__t,__x),__ARGLIST_8(__t,__VA_ARGS__)
#define ARGLIST(__t, ...) CONCAT(__ARGLIST_,ARGS_LEN(__VA_ARGS__))(__t,__VA_ARGS__)

#define __ARGREPEAT_1(__s, __x) __x __s
#define __ARGREPEAT_2(__s, __x) __ARGREPEAT_1(__s,__x) __x
#define __ARGREPEAT_3(__s, __x) __ARGREPEAT_1(__s,__x) __ARGREPEAT_2(__s,__x)
#define __ARGREPEAT_4(__s, __x) __ARGREPEAT_1(__s,__x) __ARGREPEAT_3(__s,__x)
#define __ARGREPEAT_5(__s, __x) __ARGREPEAT_1(__s,__x) __ARGREPEAT_4(__s,__x)
#define __ARGREPEAT_6(__s, __x) __ARGREPEAT_1(__s,__x) __ARGREPEAT_5(__s,__x)
#define __ARGREPEAT_7(__s, __x) __ARGREPEAT_1(__s,__x) __ARGREPEAT_6(__s,__x)
#define __ARGREPEAT_8(__s, __x) __ARGREPEAT_1(__s,__x) __ARGREPEAT_7(__s,__x)
#define __ARGREPEAT_9(__s, __x) __ARGREPEAT_1(__s,__x) __ARGREPEAT_8(__s,__x)
#define ARGREPEAT(__s, __x, ...) CONCAT(__ARGREPEAT_,ARGS_LEN(__VA_ARGS__))(__s,__x)

#define __ARGCONCAT_1(__s, __x) __x __s
#define __ARGCONCAT_2(__s, __x, ...) __ARGCONCAT_1(__s,__x) __VA_ARGS__
#define __ARGCONCAT_3(__s, __x, ...) __ARGCONCAT_1(__s,__x) __ARGCONCAT_2(__s,__VA_ARGS__)
#define __ARGCONCAT_4(__s, __x, ...) __ARGCONCAT_1(__s,__x) __ARGCONCAT_3(__s,__VA_ARGS__)
#define __ARGCONCAT_5(__s, __x, ...) __ARGCONCAT_1(__s,__x) __ARGCONCAT_4(__s,__VA_ARGS__)
#define __ARGCONCAT_6(__s, __x, ...) __ARGCONCAT_1(__s,__x) __ARGCONCAT_5(__s,__VA_ARGS__)
#define __ARGCONCAT_7(__s, __x, ...) __ARGCONCAT_1(__s,__x) __ARGCONCAT_6(__s,__VA_ARGS__)
#define __ARGCONCAT_8(__s, __x, ...) __ARGCONCAT_1(__s,__x) __ARGCONCAT_7(__s,__VA_ARGS__)
#define __ARGCONCAT_9(__s, __x, ...) __ARGCONCAT_1(__s,__x) __ARGCONCAT_8(__s,__VA_ARGS__)
#define ARGCONCAT(__s, ...) CONCAT(__ARGCONCAT_,ARGS_LEN(__VA_ARGS__))(__s,__VA_ARGS__)
#endif

#define __GCCATTR(x) __attribute__((x))
#define GCCATTRCTOR __GCCATTR(constructor)
#define GCCATTRCLEANUP(fn) __GCCATTR(__cleanup__(fn))

#endif //C4FUN_MACRO_H
