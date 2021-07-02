#ifndef MONITOR_TASK_H
#define MONITOR_TASK_H

#include <thread>
#include <exception>
#include <mutex>
#include <deque>
#include <condition_variable>
#include <atomic>

#include <unistd.h>
#include <sys/eventfd.h>

#include "pipe.h"

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

    class Runner {
    public:

        virtual ~Runner() {
            if (runner_.joinable()) {
                runner_.join();
            }
//            (void) notifier_.retrieve();
        }

        void start() {
            runner_ = std::thread([this] { Run(); });
        }

        void stop() {
            notifier_.notify(Notifier::kNotifyQuit);
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

    struct Task {
        Task() : fn{}, arg{} {}

        void *(*fn)(void *);

        void *arg;
        TaskHook hook;
    };

    class TaskRunner : public Runner {
    public:
        enum class State : char {
            IDLE,
            WORK,
        };

//        TaskRunner(std::mutex &mtx, std::condition_variable &cond) : mtx_(mtx), cond_(cond), state_(State::IDLE) {}
        TaskRunner() : state_(State::IDLE) {}

        ~TaskRunner() override = default;

        void post(const Task &task);

        bool idle() const;

    protected:
        void Run() override;

    private:
/*        std::mutex &mtx_;
        std::condition_variable &cond_;*/
        std::deque<Task> tasks_;
        std::atomic<State> state_;
        std::mutex mtx_;
    };
}

#endif //MONITOR_TASK_H
