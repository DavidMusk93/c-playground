#include "macro.h"

#define LASTCHAR 'Z'
int a[LASTCHAR];

GCCATTRCTOR void reverse_table(){
#define TABLESIZE 32
    char table[TABLESIZE]={
            '0','1','2','3','4','5','6','7','8','9',
            'B','C','D','E','F','G','H','J','K','L','M',
            'N','P','Q','R','S','T','V','W','X','Y',LASTCHAR,
    };
    int i;
    for(i=0;i<LASTCHAR;++i){
        a[i]=-1;
    }
    for(i=0;i<TABLESIZE;++i){
        a[table[i]]=i;
    }
#undef LASTCHAR
}

int id_decode(const char*p,unsigned l){
    if(--l){
        return id_decode(p,l)*TABLESIZE+a[p[l]];
    }
    return a[*p];
#undef TABLESIZE
}

int check(const char**pp){
#define PREFIXLEN 2
#define NIDLEN 4
#define PIDLEN 6
#define NAMELEN PREFIXLEN+NIDLEN+PIDLEN
    const char*p=*pp;
    static char buf[NAMELEN]="$";
    const char*e;
    int l=(int)strlen(p);
    if(*p!='$'){ /*p may not start with '$'*/
        if(l+1!=NAMELEN){
            goto err;
        }
        memcpy(buf+1,p,NAMELEN-1);
        p=*pp=buf;
    }else if(l!=NAMELEN){
        goto err;
    }
    if(p[1]!='Z'){
        goto err;
    }
    e=p+NAMELEN;
    for(p+=PREFIXLEN;p<e;++p){
        if(a[*p]==-1){
            goto err;
        }
    }
    return 0;
err:
    return 1;
#undef NAMELEN
}

MAIN_EX(argc,argv){
    const char*p=argv[1];
    if(argc==1){
        LOG_ERROR("missing name");
        goto err;
    }
    if(check(&p)){
        LOG_ERROR("invalid name");
        goto err;
    }
    LOG_INFO("nid,%d;pid,%d",id_decode(p+PREFIXLEN,NIDLEN),id_decode(p+PREFIXLEN+NIDLEN,PIDLEN));
    return 0;
err:
    return 1;
}