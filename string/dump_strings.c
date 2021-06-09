#include "macro.h"

#include <unistd.h>

const char*dump_strings(char**args){
    static char buf[512];
    char*p=&buf[0];
    char*e=buf+sizeof(buf);
    p+=sprintf(p,"args:");
    while(*args){
        if(p>=e){
            break;
        }
        p+=sprintf(p,"\"%s\",",*args);
        args++;
    }
    return &buf[0];
}

MAIN_EX(argc,argv){
    LOG("%s",dump_strings(__environ));
    LOG("%s",dump_strings((char**)argv));
    return 0;
}