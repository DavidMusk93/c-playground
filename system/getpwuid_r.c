//
// Created by Steve on 11/1/2020.
//

#include "macro.h"

#include <pwd.h>
#include <unistd.h>

MAIN(){
    struct passwd pwd,*result;
    char *buf;
    size_t size;
    int s;
    size=sysconf(_SC_GETPW_R_SIZE_MAX);
    if(size==-1){
        size=16384;
    }
    buf=malloc(size);
    s=getpwuid_r(getuid(),&pwd,buf,size,&result);
#define R(x) pwd.pw_##x
    if(!s){
        LOG("%s,%s,%d,%d,%s,%s,%s",R(name),R(passwd),(int)R(uid),(int)R(gid),R(gecos),R(dir),R(shell));
    }
    free(buf);
#undef R
}