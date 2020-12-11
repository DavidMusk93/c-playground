//
// Created by Steve on 11/22/2020.
//

#include "macro.h"

#define OFFSET(type,name) (size_t)(&((type*)0)->name)

#define _ARGS_17(_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,...) _16
#define ARGS_LEN(...) _ARGS_17(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)

#define _CONCAT(x,y) x##y
#define CONCAT(x,y) _CONCAT(x,y)

#define OFFSET_LIST_1(type,x) OFFSET(type,x)
#define OFFSET_LIST_2(type,x,...) OFFSET(type,x),OFFSET_LIST_1(type,__VA_ARGS__)
#define OFFSET_LIST_3(type,x,...) OFFSET(type,x),OFFSET_LIST_2(type,__VA_ARGS__)
#define OFFSET_LIST_4(type,x,...) OFFSET(type,x),OFFSET_LIST_3(type,__VA_ARGS__)
#define OFFSET_LIST_5(type,x,...) OFFSET(type,x),OFFSET_LIST_4(type,__VA_ARGS__)
#define OFFSET_LIST_6(type,x,...) OFFSET(type,x),OFFSET_LIST_5(type,__VA_ARGS__)
#define OFFSET_LIST_7(type,x,...) OFFSET(type,x),OFFSET_LIST_6(type,__VA_ARGS__)
#define OFFSET_LIST(type,...) CONCAT(OFFSET_LIST_,ARGS_LEN(__VA_ARGS__))(type,__VA_ARGS__)

#define FMT_1(x) ",%zu"
#define FMT_2(x,...) ",%zu" FMT_1(__VA_ARGS__)
#define FMT_3(x,...) ",%zu" FMT_2(__VA_ARGS__)
#define FMT_4(x,...) ",%zu" FMT_3(__VA_ARGS__)
#define FMT_5(x,...) ",%zu" FMT_4(__VA_ARGS__)
#define FMT_6(x,...) ",%zu" FMT_5(__VA_ARGS__)
#define FMT_7(x,...) ",%zu" FMT_6(__VA_ARGS__)
#define FMT(...) CONCAT(FMT_,ARGS_LEN(__VA_ARGS__))(__VA_ARGS__)

struct A{
    int*p;
    short x,y;
};

typedef struct _S1{
    char a;
    char b;
    double c;
} S1;

#pragma pack(push,8)
typedef struct _S2{
    char c;
    double d;
    short s;
    char a[9];
    int i;
} S2;

typedef struct _S3{
    char c;
    S2 st;
    char a[10];
    int i;
} S3;

typedef struct _S4_8{
    int i;
    double d;
    char*p;
    char c;
    int*q;
} S4_8;
#pragma pack(pop)
#pragma pack(push,4)
typedef struct _S4_4{
    int i;
    double d;
    char*p;
    char c;
    int*q;
} S4_4;
#pragma pack(pop)
//MAIN(){
#define DUMP(type,...) LOG(#type ":%zu" FMT(__VA_ARGS__),sizeof(type),OFFSET_LIST(type,__VA_ARGS__))
//    DUMP(S1,a,b,c);
//    DUMP(S2,c,d,s,a,i);
//    DUMP(S3,c,st,a,i);
//    DUMP(S4_8,i,d,p,c,q);
//    DUMP(S4_4,i,d,p,c,q);
//}

MAIN(){
    DUMP(struct A,p,x,y);
}