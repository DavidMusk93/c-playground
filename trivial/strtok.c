//
// Created by Steve on 9/14/2020.
//

#include "macro.h"

MAIN(){
#define DELIM " "
    char a[]="a"DELIM"b";
    char b[]="b"DELIM"c";
    char c[]="c"DELIM"d";
    char*pairs[]={a,b,c};
    for(int i=0,n=dimension_of(pairs);i<n;++i){
        char*k=strtok(pairs[i],DELIM); /*@NOTICE loop restart*/
        char*v=strtok(NULL,DELIM);
        LOG("%s,%s",k,v);
    }
}