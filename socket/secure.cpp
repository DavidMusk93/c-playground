//
// Created by Steve on 8/27/2020.
//

#include "secure.h"

#include <cstring>

using u8=unsigned char;

std::string Secure::Encrypt(const void *data, size_t len/*8 for now*/) {
#define RANDOM 4
    std::string res=SECRET;
    res.resize(kDataLen);
    res[SECRET_LEN-1]=RANDOM;
    memcpy(&res[SECRET_LEN],data,len);
//    Ring<kDataLen-1> ring;
//    void*p=ring.next();
    auto p=reinterpret_cast<u8*>(&res[0]);
    auto mask=p[RANDOM];
    size_t i=RANDOM+1;
    for(;i!=RANDOM;++i){
        if(i==kDataLen){
            i=0;
        }
        if(i+1!=SECRET_LEN){
            p[i]^=mask;
        }
    }
    return res;
}

bool Secure::Decrypt(std::string &s, void *data, size_t len) {
    if(s.size()==kDataLen){
        auto p=reinterpret_cast<u8*>(&s[0]);
        const auto R=p[SECRET_LEN-1];
        if(R>=kDataLen){
            return false;
        }
        auto mask=p[R];
        size_t i=R+1;
        for(;i!=R;++i){
            if(i==kDataLen){
                i=0;
            }
            if(i+1!=SECRET_LEN){
                p[i]^=mask;
            }
        }
        if(strncmp(&s[0],SECRET,SECRET_LEN-1)==0){
            memcpy(data,&s[SECRET_LEN],len);
            return true;
        }
    }
    return false;
}

//int main(){
//    long l=0x2345634523;
//    long r{};
//    auto s=Secure::Encrypt(&l,sizeof(l));
//    if(Secure::Decrypt(s,&r,sizeof(r))){
//        printf("%lx,%lx\n",l,r);
//    }else{
//        puts("UNKNOWN");
//    }
//    auto buf=Secure::CreateBuffer();
//    printf("buffer length: %ld\n",buf.size());
//    l=12;
//    s=Secure::Encrypt(&l,sizeof(l));
//    Secure::Decrypt(s,&r,sizeof(r));
//    printf("%lx,%lx\n",l,r);
//}