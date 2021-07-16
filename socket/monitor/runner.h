#ifndef MONITOR_TASK_H
#define MONITOR_TASK_H

#include <thread>
#include <exception>
#include <mutex>
#include <deque>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <unordered_set>
#include <bitset>

#include <unistd.h>
#include <sys/eventfd.h>

#include "pipe.h"
#include "state.h"
#include "timer.h"
#include "util.h"

#define LOCKGUARD(x) std::lock_guard<std::mutex> _lg(x)
#define UNIQUELOCK(x, ...) std::unique_lock<std::mutex> _ul(x,##__VA_ARGS__)

namespace sun {
    class Terminator {
    public:
        Terminator() = default;

        ~Terminator() = default;

        int fd() const {
            return pipe_.readEnd();
        }

        void trigger() {
            char c = 'Q';
            write(pipe_.writeEnd(), &c, 1);
        }

        void cleanup() const {
            char c;
            read(fd(), &c, 1);
        }

    private:
        Pipe pipe_;
    };

    class Notifier : public transferable {
    public:
        explicit Notifier(int flags = 0) {
            fd_ = eventfd(0, flags);
        }

        ~Notifier() override {
            if (fd_ != -1) {
                close(fd_);
                fd_ = -1;
            }
        }

        void notify(eventfd_t event) {
            if (fd_ != -1) {
                (void) write(fd_, &event, sizeof event);
            }
        }

        eventfd_t retrieve() {
            eventfd_t event{};
            if (fd_ != -1) {
                (void) read(fd_, &event, sizeof event);
            }
            return event;
        }

        static constexpr const eventfd_t kNotifyQuit = 0x1ffffffffUL;
        static constexpr const eventfd_t kNotifyOne = 1;
    };

    template<size_t N>
    class IdManager {
    public:
        IdManager() {
            static_assert(N < 102400, "maximum id is too large");
        }

        int alloc() {
            LOCKGUARD(mtx_);
            for (size_t i = 0; i < N; ++i) {
                if (!set_.test(i)) {
                    set_.set(i);
                    return (int) i;
                }
            }
            return -1;
        }

        void dealloc(int id) {
            if (id >= 0 && id < N) {
                LOCKGUARD(mtx_);
                set_.reset(id);
            }
        }

        bool valid(int id) {
            if (id >= 0 && id < N) {
                LOCKGUARD(mtx_);
                return set_.test(id);
            }
            return false;
        }

    private:
        std::bitset<N> set_;
        std::mutex mtx_;
    };

    class Runner : public stateful {
    public:

        virtual ~Runner() {
            if (!initialized()) {
                return;
            }
            if (runner_.joinable()) {
                runner_.join();
            }
//            (void) notifier_.retrieve();
        }

        void start() {
            if (initialized()) {
                return;
            }
            runner_ = std::thread([this] { Run(); });
            while (getstate() != State::IDLE); /*wait for thread start*/
        }

        void stop() {
            notifier_.notify(Notifier::kNotifyQuit);
            setstate(State::TERMINATED);
        };

        int notifier() const {
            return notifier_.fd();
        }

    protected:
        virtual void Run() = 0;

    protected:
        Notifier notifier_;

    private:
        std::thread runner_;
    };

    struct TaskHook {
        std::function<void(void *)> oncall; /*hook input*/
        std::function<void(const void *)> onreturn; /*hook output*/
        std::function<void(const std::exception &)> onexception;
    };

    typedef void *(*fn_job_t)(void *);

    struct Task {
        Task() : id(-1), fn{}, arg{} {}

        void run() const;

        mutable int id;
        fn_job_t fn;
        void *arg;
        std::function<void(void *)> dispose; /*use to free arg*/
        Closure closure;
        TaskHook hook; /*easy to use but each functor consumes 32 bytes*/
        static void *kClosureTag;
    };

    class TaskRunner : public Runner {
    public:
//        TaskRunner(std::mutex &mtx, std::condition_variable &cond) : mtx_(mtx), cond_(cond), state_(State::IDLE) {}
        TaskRunner() = default;

        ~TaskRunner() override = default;

        void post(const Task &task);

        bool idle() const;

    protected:
        void Run() override;

    private:
//        std::mutex &mtx_;
//        std::condition_variable &cond_;
        std::deque<Task> tasks_;
        std::mutex mtx_;
    };

    enum {
        TIMERTASK_ONCE,
        TIMERTASK_REPEATED,
    };

    struct tTask : Task {
        using tCmp = std::function<bool(const tTask &t1, const tTask &t2)>;

        tTask() : runat(-1), duration(0), type(-1) {}

        tTask(unsigned duration, int type, fn_job_t fn, void *arg, std::function<void(void *)> dispose = {})
                : runat(util::Milliseconds() + duration),
                  duration(duration), type(type) {
            this->fn = fn, this->arg = arg;
            if (dispose) {
                this->dispose.swap(dispose);
            }
        }

        tTask(unsigned duration, int type, Closure closure)
                : runat(util::Milliseconds() + duration),
                  duration(duration), type(type) {
            arg = kClosureTag;
            this->closure.swap(closure);
        }

        double runat;
        unsigned duration;
        int type;

        static tCmp Greater;
        static tTask Nil;
        static constexpr const unsigned kTypeMask = 0xff;
        static constexpr const unsigned kCancelMask = 0x100;
    };

//    bool tTaskGreater(const tTask &t1, const tTask &t2);

    class tRunner : public Runner {
    public:
        using tHeap = std::priority_queue<tTask, std::deque<tTask>, tTask::tCmp>;

        tRunner() : timer_({}), tasks_(tTask::Greater), id_(0) {}

        ~tRunner() override = default;

        int post(const tTask &task);

        void cancel(int id);

        int fd() const {
            return timer_.fd();
        }

    protected:
        void Run() override;

    private:
        Timer timer_;
        tHeap tasks_;
        int id_;
        std::unordered_set<int> marks_;
        std::mutex mtx_;
        std::mutex mark_mtx_;
    };
}

#endif //MONITOR_TASK_H
