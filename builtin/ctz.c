//
// Created by Steve on 11/16/2020.
// @ref https://stackoverflow.com/questions/7812044/finding-trailing-0s-in-a-binary-number
//

#include "macro.h"

static unsigned __ctz(unsigned x){
    if(x){
        x&=-x;
        unsigned c=(sizeof(x)<<3)-1;
        if(x&0x0000ffff){c-=16;}
        if(x&0x00ff00ff){c-=8;}
        if(x&0x0f0f0f0f){c-=4;}
        if(x&0x33333333){c-=2;}
        if(x&0x55555555){c-=1;}
        return c;
    }
    return -1; /*undefined*/
}

static const char*int_to_bin(unsigned x,char buf[40]);
static unsigned __ctz2(unsigned x){
    if(x){
        --x;
        LOG("@STAGE-1 %s",int_to_bin(x,0));
        x-=(x>>1)&0x55555555;
        LOG("@STAGE-2 %s",int_to_bin(x,0));
        x=(x&0x33333333)+((x>>2)&0x33333333);
        LOG("@STAGE-3 %s",int_to_bin(x,0));
        x=(x+(x>>4))&0x0f0f0f0f;
        LOG("@STAGE-4 %s",int_to_bin(x,0));
        x+=x>>8;
        LOG("@STAGE-5 %s",int_to_bin(x,0));
        x+=x>>16;
        LOG("@STAGE-6 %s",int_to_bin(x,0));
        return x&0x1f;
        return x & 0x3f;
    }
    return -1;
}

static const char*g_bin_table[]={
        "0000","0001","0010","0011",
        "0100","0101","0110","0111",
        "1000","1001","1010","1011",
        "1100","1101","1110","1111",
};

static const char*int_to_bin(unsigned x,char buf[40]){
    static __thread char __buf[40];
    char*p=buf?:__buf;
    static const char*fmt[]={"%s","%s'"};
    for(unsigned i=28;;i-=4){
        p+=sprintf(p,fmt[!!i],g_bin_table[(x>>i)&0xf]);
        if(!i){
            break;
        }
    }
    return buf?:__buf;
}

MAIN(){
    unsigned k;
//    for(unsigned i=0;i<32;++i){
//        k=(1<<i);
//        LOG("%#x,%u",k,__ctz(k));
//    }
    k=0x1f00;
    LOG("%#x,%u",k,__ctz2(k));
}
