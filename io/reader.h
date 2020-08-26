//
// Created by Steve on 8/26/2020.
//

#ifndef C4FUN_READER_H
#define C4FUN_READER_H

class Reader{
public:
    Reader():fd_(-1){}
    virtual ~Reader()=default;

    virtual bool Eof()=0;
    virtual short GetShort()=0;
    virtual int GetInt()=0;
    virtual int GetObserveFd()=0;

protected:
    int&GetFd(){
        return fd_;
    }

private:
    int fd_;
};

#define LOG(fmt,...) printf(fmt "\n",##__VA_ARGS__)

#define ERROR_RETURN(error,code,block,fmt,...) \
do{\
    if(error){\
        LOG(fmt,##__VA_ARGS__);\
        block\
        return code;\
    }\
}while(0)

#define ERROR_RETURN4(expr,code,cleanup,err) \
do{\
    if(expr){\
        cleanup\
        LOG("%s: %s",#expr,sizeof(#err)==1?"":ERROR_S);\
        return code;\
    }\
}while(0)

#define SETSOCKOPT(sock,level,name,val,code) \
ERROR_RETURN(\
setsockopt(sock,level,name,&val,sizeof(val))!=0,\
code,\
{close(sock);sock=-1;},\
"setsockopt(" #name ") error: %s",ERROR_S)

#define ERROR_S strerror(errno)

#endif //C4FUN_READER_H
