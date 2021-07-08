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

#include <unistd.h>
#include <sys/eventfd.h>

#include "pipe.h"
#include "state.h"
#include "timer.h"
#include "util.h"

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
            setstate(State::INITIALIZED);
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
        Task() : fn{}, arg{} {}

        void run() const;

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
/*        std::mutex &mtx_;
        std::condition_variable &cond_;*/
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
    };

//    bool tTaskGreater(const tTask &t1, const tTask &t2);

    class tRunner : public Runner {
    public:
        using tHeap = std::priority_queue<tTask, std::deque<tTask>, tTask::tCmp>;

        tRunner() : timer_({}), tasks_(tTask::Greater) {}

        ~tRunner() override = default;

        void post(const tTask &task);

        int fd() const {
            return timer_.fd();
        }

    protected:
        void Run() override;

    private:
        Timer timer_;
        tHeap tasks_;
        std::mutex mtx_;
    };
}

#endif //MONITOR_TASK_H
