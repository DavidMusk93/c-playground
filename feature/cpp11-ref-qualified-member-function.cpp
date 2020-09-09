//
// Created by Steve on 9/7/2020.
//

#include <algorithm>

#include "macro.h"

struct Bar{
    Bar&&move(){
        return static_cast<Bar&&>(*this);
    }
};

#define MARK() LOG("'%s' called",__PRETTY_FUNCTION__)

struct Foo{
    Bar getBar()&{MARK();return bar;}
    Bar getBar()const&{MARK();return bar;}
    Bar getBar()&&{MARK();return bar.move();}
    Bar getBar()const&&{MARK();return std::move(bar);}

private:
    Bar bar;
};

MAIN(){
    Foo foo{};
    Bar bar=foo.getBar();
    const Foo foo2{};
    Bar bar2=foo2.getBar();
    Foo{}.getBar();
    std::move(foo).getBar();
    std::move(foo2).getBar();
}