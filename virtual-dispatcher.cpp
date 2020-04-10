//
// Created by Steve on 4/10/2020.
//

#include <iostream>
#include <vector>

class A {
public:
    virtual ~A() = default;

    virtual void foo() = 0;
};

class B : public A {
public:
    explicit B(int x) : x_(x) {}

    void foo() override {
        std::cout << "integer: " << x_ << std::endl;
    }

private:
    int x_;
};

class C : public A {
public:
    explicit C(float x) : x_(x) {}

    void foo() override {
        std::cout << "float: " << x_ << std::endl;
    }

private:
    float x_;
};

int main() {
    std::vector<A *> data;
    data.push_back(new B(2));
    data.push_back(new C(0.2));
    for (auto &a:data) {
        if (dynamic_cast<B *>(a)) {
            std::cout << "class B\n";
        } else if (dynamic_cast<C *>(a)) {
            std::cout << "class C\n";
        }
        a->foo();
        delete (a);
    }
    return 0;
}