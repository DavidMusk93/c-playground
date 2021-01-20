#ifndef MONITOR_BASE_H
#define MONITOR_BASE_H

#include <functional>

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <error.h>

#include "fs.h"
#include "util.h"

#define WHEREFALSE while(false)
#define INITMEMBER(x) x##_(x)

#define __LOG(os, fmt, ...) fprintf(os,"%s " fmt "\n",sun::utility::Now(nullptr),##__VA_ARGS__)
#define LOGINFO(fmt, ...) __LOG(stdout,fmt,##__VA_ARGS__)
#define LOGERROR(fmt, ...) __LOG(stderr,fmt,##__VA_ARGS__)
#define FUNCLOG(fmt, ...) LOGINFO("@%s " fmt,__func__,##__VA_ARGS__)

#define ERRNOSTR strerror(errno)
#define __ATTR(x) __attribute__((x))

#define POLL(__rval, __fn, ...) \
__rval=__fn(__VA_ARGS__);\
if(__rval<0){\
    if(errno==EINTR||errno==EAGAIN){\
        continue;\
    }\
    LOGERROR(#__fn "=%d: %s",__rval,ERRNOSTR);\
    break;\
}

#define ERRRET(expr, code, block, issyscall, fmt, ...) \
if(expr){\
    block\
    (void)(issyscall?LOGERROR(fmt ": %s",##__VA_ARGS__,ERRNOSTR):LOGERROR(fmt,##__VA_ARGS__));\
    return code;\
}

namespace sun {
    class Defer {
    public:
        using Closure = std::function<void(void)>;

        Defer() = default;

        explicit Defer(Closure closure) {
            closure_.swap(closure);
        }

        Defer(Defer &&defer) noexcept {
            *this = defer.move();
        }

        ~Defer() {
            if (closure_) {
                closure_();
            }
        }

        Defer &operator=(Defer &&defer) noexcept {
            closure_.swap(defer.closure_);
            return *this;
        }

        Defer &&move() {
            return static_cast<Defer &&>(*this);
        }

        void cancel() {
            Closure trivial;
            closure_.swap(trivial);
        }

        void trigger() {
            if (closure_) {
                Closure trivial;
                closure_();
                closure_.swap(trivial);
            }
        }

        void swap(Defer &defer) {
            closure_.swap(defer.closure_);
        }

    private:
        Closure closure_;
    };

    class nocopy {
    public:
        nocopy(const nocopy &) = delete;

        nocopy &operator=(const nocopy &) = delete;

    protected:
        nocopy() = default;

        ~nocopy() = default;
    };

    namespace utility {
        int getpid();
    }
}

#endif //MONITOR_BASE_H
