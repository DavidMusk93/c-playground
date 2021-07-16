#include "macro.h"

class A {
public:
    A(int x):x_(x){}

    static void Foo(void*arg);

private:
    void a() {
        LOG("call private method,%d",x_);
    }
    int x_;
};

void A::Foo(void *arg) {
    auto*self=(A*)(arg);
    self->a();
}

MAIN(){
    A a(2);
    A::Foo(&a);
}