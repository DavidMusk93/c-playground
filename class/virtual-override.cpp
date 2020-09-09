//
// Created by Steve on 9/8/2020.
//

#include "macro.h"

#include <iostream>
#include <ostream>

struct Base{
    virtual Base*get(){
        return this;
    }
    virtual bool equal(Base*other){
        return i==other->i;
    }
    virtual std::ostream&print(int n,std::ostream&os=std::cout){
        os<<n<<std::endl;
        return os;
    }
    int i{};
};

struct Derived:public Base{
    Derived*get()override{
        return this;
    }
    bool equal(Derived*other){
        return false;
    }
    std::ostream&print(int n,std::ostream& os)override{
        os<<n<<","<<n<<std::endl;
        return os;
    }
};

MAIN(){
    Derived derived{};
    Derived derived2{};
    Base*ptr=new Derived();
    ptr->print(1);
    derived.print(1,std::cout);
    LOG("%d,%d",ptr->equal(&derived),derived.equal(&derived2));
    delete ptr;
}