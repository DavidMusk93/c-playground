#include "runner.h"

#include <vector>

#include <sys/poll.h>

namespace sun {
    void TaskRunner::post(const Task &task) {
        if (task.fn) {
            mtx_.lock();
            tasks_.push_front(task);
            mtx_.unlock();
            notifier_.notify(Notifier::kNotifyOne);
//            cond_.notify_one();
        }
    }

    void TaskRunner::Run() {
        struct pollfd pfd{.fd=notifier_.fd(), .events=POLLIN};
        eventfd_t e;
        int rc;
        for (;;) {
            POLL(rc, poll, &pfd, 1, -1);
            e = notifier_.retrieve();
            if (e >= Notifier::kNotifyQuit) {
                break;
            }
            LOGDEBUG("@TASKRUNNER posted tasks: %lu", e);
            std::vector<Task> tasks;
            mtx_.lock();
            while (e--) {
                tasks.push_back(tasks_.front());
                tasks_.pop_front();
            }
            mtx_.unlock();
            state_.store(State::WORK, std::memory_order_release);
            for (auto &&task:tasks) {
#define CALL(fn, ...) if(fn){fn(__VA_ARGS__);}
                CALL(task.hook.oncall, task.arg);
                void *p{};
                try {
                    p = task.fn(task.arg);
                } catch (std::exception &ex) {
                    CALL(task.hook.onexception, ex);
                }
                CALL(task.hook.onreturn, p);
#undef CALL
            }
            state_.store(State::IDLE, std::memory_order_release);
        }
    }

    bool TaskRunner::idle() const {
        return state_.load(std::memory_order_acquire) == State::IDLE;
    }
}