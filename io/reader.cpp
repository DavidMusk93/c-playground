//
// Created by Steve on 8/26/2020.
//

#include "reader.h"

#include <string>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

class RandomReader:public Reader{
public:
    RandomReader(const std::string&file):Reader(),i_(kBufferSize){
        int fd=-1;
        ERROR_RETURN((fd=open(file.c_str(),O_RDONLY))==-1,,,"fail to open %s: %s",file.c_str(),strerror(errno));
        GetFd()=fd;
    }

    ~RandomReader() override{
        close(GetFd());
    }

    bool Eof() override {
        return false;
    }

    short GetShort() override {
        short x{};
        Fetch(&x,sizeof x);
        return x;
    }

    int GetInt() override {
        return 0;
    }

    int GetObserveFd() override {
        return 0;
    }

protected:
    void Fetch(void *data,size_t len){
        if(len){
            auto p=reinterpret_cast<char*>(data);
            while(len--){
                if(i_+1>kBufferSize){
                    i_=0;
                    read(GetFd(),buf_,kBufferSize);
                }
                *p++=buf_[i_++];
            }
        }
    }
private:
    int i_;
    char buf_[1024];

    static constexpr int kBufferSize=sizeof(buf_);
};