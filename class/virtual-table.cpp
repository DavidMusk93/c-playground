//
// Created by Steve on 9/8/2020.
//

#include "macro.h"

#define WHICH() LOG("'%s' called",__PRETTY_FUNCTION__)

class Base{
public:
    virtual void a(){
        WHICH();
    }
};

class Child:public Base{
public:
    virtual void b(int&x){
        WHICH();
        x*=2;
    }
};

class Grandchild:public Child{
public:
    void c(float&f){
        WHICH();
        f*=1.1f;
    }
};

MAIN(){
    Base base{};
    Child child{};
    Grandchild grandchild{};
    Base&ref=grandchild;
    Child&ref2=grandchild;
//    Base*ptr=new Grandchild();
    LOG("%zu,%zu,%zu",sizeof(base),sizeof(child),sizeof(grandchild));
    void**p=*(void***)&base;
    LOG("@BASE VT:%p",*p);
    p=*(void***)&child;
    LOG("@CHILD VT:%p,%p",p[0],p[1]);
    p=*(void***)&grandchild;
    LOG("@GRANDCHILD VT:%p,%p",p[0],p[1]);
    int a=2;
    float f=2;
    grandchild.a();
    grandchild.b(a);
    grandchild.c(f);
//    ref.a();
//    ref2.a();
//    ref2.b(a);
    LOG("%d,%f",a,f);
}