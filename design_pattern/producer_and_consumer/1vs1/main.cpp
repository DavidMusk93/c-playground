#include <queue>
#include <thread>
#include <chrono>

#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/eventfd.h>
#include <poll.h>

static int __gettid() {
    static __thread int tid = 0;
    if (!tid) {
        tid = (int) syscall(SYS_gettid);
    }
    return tid;
}

static const char *nowstr() {
    static __thread char buffer[64];
    struct timeval tv{};
    gettimeofday(&tv, nullptr);
    sprintf(buffer, "%ld.%06ld", tv.tv_sec, tv.tv_usec);
    return buffer;
}

enum Event {
    QUEUEREADY = 1,
    QUIT = 0x1fffff,
};

int notifier;
long pid;
#define LOG(fmt, ...) printf("%ld,%ld %s %s:%d " fmt "\n",pid,(long)__gettid(),nowstr(),__PRETTY_FUNCTION__,__LINE__,##__VA_ARGS__)

class TrivialMutex {
public:
    TrivialMutex() {
        LOG("CTOR");
        pthread_mutex_init(&mtx_, nullptr);
    }

    ~TrivialMutex() {
        LOG("DTOR");
        pthread_mutex_destroy(&mtx_);
    }

    pthread_mutex_t &Get() {
        return mtx_;
    }

private:
    pthread_mutex_t mtx_{};
};

void trivial_mutex_destroy(void *ptr) {
    LOG("CALLED,%p", ptr);
    delete reinterpret_cast<TrivialMutex *>(ptr);
}

pthread_key_t key;

static __attribute__((constructor)) void ctor() {
    notifier = eventfd(0, 0);
    pid = getpid();
    LOG("GLOBAL CTOR");
    pthread_key_create(&key, &trivial_mutex_destroy);
}

static __attribute__((destructor)) void dtor() {
    LOG("GLOBAL DTOR");
    pthread_key_delete(key);
    close(notifier);
}

class TrivialLocal {
public:
    TrivialLocal() : val_(nullptr) {
        LOG("CTOR");
    }

    ~TrivialLocal() {
        LOG("DTOR");
    }

    void *Get() {
        val_ = pthread_getspecific(key);
        if (val_) {
            return val_;
        }
        val_ = new TrivialMutex;
        pthread_setspecific(key, val_);
        return val_;
    }

private:
    void *val_;
};

template<class T>
class BlockQueue {
public:
    class LockGuard {
    public:
        explicit LockGuard(pthread_mutex_t &mtx) : mtx_(mtx) {
            pthread_mutex_lock(&mtx_);
        }

        ~LockGuard() {
            pthread_mutex_unlock(&mtx_);
        }

    private:
        pthread_mutex_t &mtx_;
    };

    BlockQueue() {
        LOG("CTOR");
        pthread_mutex_init(&mtx_, nullptr);
        pthread_cond_init(&cond_, nullptr);
    }

    ~BlockQueue() {
        LOG("DTOR");
        pthread_cond_destroy(&cond_);
        pthread_mutex_destroy(&mtx_);
    }

    void put(T t) {
        LockGuard lg{mtx_};
        q_.push(t);
        pthread_cond_broadcast(&cond_);
        int64_t event = QUEUEREADY;
        write(notifier, &event, sizeof(event));
    }

    T get() {
        LockGuard lg{mtx_};
        auto t = q_.front();
        q_.pop();
        return t;
    }

    bool ready() {
        LockGuard lg{mtx_};
        while (q_.empty()) {
            int rc = pthread_cond_wait(&cond_, &mtx_);
            if (rc) {
                return false;
            }
        }
        return true;
    }

    size_t size() {
        return q_.size();
    }

private:
    pthread_mutex_t mtx_{};
    pthread_cond_t cond_{};
    std::queue<T> q_;
};

BlockQueue<int> queue;

void exit_handler() {
    LOG("EXIT");
}

int main() {
    atexit(&exit_handler);
    TrivialLocal local;
    auto producer = std::thread([&local] {
//        TrivialMutex trivial;
        LOG("@PRODUCER BEFORE %p", local.Get());
        for (int i = 0; i < 10; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            queue.put(i * i);
        }
        LOG("@PRODUCER AFTER %p", local.Get());
    });
    auto spoiler = std::thread([&local] {
//        TrivialMutex trivial;
        LOG("@SPOILER BEFORE %p", local.Get());
        std::this_thread::sleep_for(std::chrono::seconds(5));
//        exit(0);
        int64_t event = QUIT;
        write(notifier, &event, sizeof event);
        LOG("@SPOILER AFTER %p", local.Get());
    });
//    spoiler.detach();
    LOG("@CONSUMER BEFORE %p", local.Get());
    struct pollfd pfd{.fd=notifier, .events=POLLIN};
    while (poll(&pfd, 1, -1) != -1) {
        int64_t event{};
        read(notifier, &event, sizeof event);
        if (event < QUIT) {
            auto t = queue.get();
            printf("@CONSUME %d\n", t);
        } else {
            break;
        }
    }
//    while (queue.ready()) {
//        auto t = queue.get();
//        printf("@CONSUME %d\n", t);
//    }
    spoiler.join();
    producer.join();
    LOG("@CONSUMER AFTER %p", local.Get());
}