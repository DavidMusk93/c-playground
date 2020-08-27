//
// Created by Steve on 8/27/2020.
//

#ifndef C4FUN_SECURE_H
#define C4FUN_SECURE_H

#include <functional>
#include <string>

template<size_t N>
struct Ring{
public:
    void*next(){
        void*p=data[i];
        if(++i==N){
            i=0;
        }
        return p;
    }

private:
    size_t i{};
    void*data[N]{};
};

class Secure{
public:
    static std::string Encrypt(const void*data, size_t len);
    static bool Decrypt(std::string&s,void*data,size_t len);
    static std::string CreateBuffer(){
        return std::string(kDataLen,0);
    }

private:
    static constexpr const size_t kDataLen=4+8;
    static constexpr const char*SECRET="SUN";
    static constexpr const size_t SECRET_LEN=4;
};

#endif //C4FUN_SECURE_H
