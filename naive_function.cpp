//
// Created by Steve on 3/14/2020.
//

#include <memory>
#include <cassert>
#include <iostream>

template<typename T>
class naive_function;

template<typename ReturnType, typename... Args>
class naive_function<ReturnType(Args...)> {
public:
    template<typename T>
    naive_function &operator=(T &&t) {
        release_();
        using value_type = CallableT<T>;
        if (sizeof(value_type) <= sizeof(buffer_)) {
            callable_ = new(buffer_)value_type(std::forward<T>(t));
        } else {
            callable_ = new value_type(std::forward<T>(t));
        }
        return *this;
    }

    explicit naive_function() : callable_(nullptr) {}

    ~naive_function() {
        release_();
    }

    ReturnType operator()(Args &&... args) const {
        assert(callable_);
        return callable_->Invoke(std::forward<Args>(args)...);
    }

private:
    class ICallable {
    public:
        virtual ~ICallable() = default;

        virtual ReturnType Invoke(Args...) = 0;
    };

    template<typename T>
    class CallableT : public ICallable {
    public:
        explicit CallableT(T &&t) : t_(std::forward<T>(t)) {}

        ~CallableT() override = default;

        ReturnType Invoke(Args... args) override {
            return t_(args...);
        }

    private:
        T t_;
    };

    void release_() {
        if (callable_ && (void *) callable_ != (void *) buffer_) {
            delete callable_;
        }
    }

    ICallable *callable_;
    char buffer_[16] = {};
};

void func() {
    std::cout << "func" << std::endl;
}

struct functor {
    void operator()() {
        std::cout << "functor" << std::endl;
    }
};

int main() {
    naive_function<void()> f;
    f = func;
    f();
    f = functor();
    f();
    f = []() { std::cout << "lambda" << std::endl; };
    f();
}