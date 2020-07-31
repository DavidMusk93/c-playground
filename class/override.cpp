//
// Created by Steve on 7/22/2020.
//

#include <stdarg.h>
#include <functional>

#include "macro.h"

#define TEST_OVERRIDE 1
#if TEST_OVERRIDE
struct ScopedVariadic{
    va_list args;
    ~ScopedVariadic(){
        va_end(args);
    }
    va_list& va(){
        return args;
    }
//    operator va_list&(){
//        return args;
//    }
};

struct Interface{
    virtual void call(void*p,...)=0;
};

struct A:Interface{
    void call(void *p, ...) override {
        ScopedVariadic sv{};
        va_start(sv.va(),p);
        LOG("@%s %p,%p,%f",__func__,this,p,va_arg(sv.va(),double));
        LOG("static Foo address: %p",&Foo);
    }
    static int Foo(void*p){
        return p?*(unsigned char*)p:0;
    }
//    int Bar(void*p){
//        LOG("%p", reinterpret_cast<void*>(&Foo)); //illegal for compiler
//        return p?*(unsigned char*)p:0;
//    }
};

typedef void (*FN_A_call)(A*,void*,...);

struct B:Interface{
    void call(void *p, ...) override {
        ScopedVariadic sv{};
        va_start(sv.va(),p);
        LOG("@%s %p,%d",__func__,p,va_arg(sv.va(),int));
    }
};

void delegate(decltype(&A::call) fn,A*obj,double d){
//    fn(obj,&d,d);
    std::invoke(fn,obj,&d,d);
}

MAIN(){
    A a{};
    B b{};
//    a.call(&a,3.12);
    b.call(&b,312);
//    LOG("%p",*(void**)&a);
//    LOG("%p,%p",&A::call,*(void**)&a);
    // VIRTUAL TABLE
    //   *It is a void* pointer at the beginning of object, vptr=*(void**)&obj;
    //   *It is a two dimensional void* table point to member functions, the first is *(void**)vptr.
    // ((FN_A_call)**(void***)&a)(&a,&a,3.12);
//    delegate(&A::call, &a, 3.12);
    auto functor_ptr=&A::call;
    (a.*functor_ptr)(&a,3.12); //check std::invoke possible implementation
}
#endif