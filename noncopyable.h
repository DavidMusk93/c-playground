//
// Created by Steve on 3/15/2020.
//

#ifndef CPP4FUN_NONCOPYABLE_H
#define CPP4FUN_NONCOPYABLE_H

namespace sun {
    class noncopyable {
    public:
        noncopyable(const noncopyable &) = delete;

        noncopyable &operator=(const noncopyable &) = delete;

    protected:
        noncopyable() = default;

        ~noncopyable() = default;
    };
}

#endif //CPP4FUN_NONCOPYABLE_H
