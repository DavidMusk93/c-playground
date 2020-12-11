//
// Created by Steve on 10/27/2020.
//

#include <iostream>
#include <string>
#include <queue>

#include "macro.h"

using namespace std;

#define MARK() LOG("'%s' invoked",__PRETTY_FUNCTION__)

class A{
public:
    A(){
        MARK();
        data_=new void*();
    }
    ~A(){
        MARK();
        delete data_;
    }
private:
    void **data_;
};

MAIN(){
    auto arr=new string*[2];
    string s("testfasddddddddadfasfkljajdlfadlfjalsdfal;fjasdlfajsdkfasdfjadasdfnjasdflasdjfalsdkfjasdfjasdfjasdlfjasdfalsdfjaskdfasd;lfjkasldfasdfjkasd;lflasdfjkasd");
    auto p=new string(s);
    arr[0]=p;
    string ss=*p;
//    cout<<s<<","<<*p<<","<<ss<<endl;
    delete p;
    LOG("sizeof string:%zu",sizeof(ss));
    p=NULL;
    delete[]arr;
    auto a=new A();
    auto b=a;
    delete b; /*call dtor & free resource*/
    LOG("END");
}