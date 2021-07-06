#include "runner.h"

#include <vector>

#include <sys/poll.h>

namespace sun {
    void Task::run() const {
#define CALL(fn, ...) if(fn){fn(__VA_ARGS__);}
        CALL(hook.oncall, arg);
        void *p{};
        try {
            p = fn(arg);
        } catch (std::exception &ex) {
            CALL(hook.onexception, ex);
        }
        CALL(hook.onreturn, p);
#undef CALL
    }

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
        setstate(State::IDLE);
        LOGINFO("@TASKRUNNER start");
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
            setstate(State::WORK);
            for (auto &&task:tasks) {
                task.run();
            }
            setstate(State::IDLE);
        }
    }

    bool TaskRunner::idle() const {
        return getstate() == State::IDLE;
    }

    tTask::tCmp tTask::Greater = [](const tTask &t1, const tTask &t2) {
        return t1.runat > t2.runat;
    };

    tTask tTask::Nil(0, 0, nullptr, nullptr);

    void tRunner::post(const tTask &task) {
        if (task.fn) {
            mtx_.lock();
            tasks_.push(task);
            mtx_.unlock();
            if (task.duration < Timer::kLeastDuration) { /*run immediately*/
                notifier_.notify(Notifier::kNotifyOne);
            } else if (getstate() == State::IDLE) { /*the first task*/
                LOGDEBUG("@TIMERRUNNER the first task");
                timer_.reset(Timer::Config{0, 0, 0, task.duration});
                setstate(State::WORK);
            } else {
                LOGDEBUG("@TIMERRUNNER check top task");
                tTask top;
                mtx_.lock();
                top = tasks_.top();
                mtx_.unlock();
                if (top.runat == task.runat) { /*top task*/
                    timer_.reset(Timer::Config{0, 0, 0, task.duration});
                }
            }
        } else { /*timeout*/
            notifier_.notify(Notifier::kNotifyOne);
        }
    }

    void tRunner::Run() {
        setstate(State::IDLE);
        LOGINFO("@TIMERRUNNER start");
        struct pollfd pfd{.fd=notifier_.fd(), .events=POLLIN};
        eventfd_t e;
        int rc;
        for (;;) {
            POLL(rc, poll, &pfd, 1, -1);
            e = notifier_.retrieve();
            if (e >= Notifier::kNotifyQuit) {
                break;
            }
            LOGDEBUG("@TIMERRUNNER posted tasks: %lu", e);
            std::vector<tTask> tasks;
            mtx_.lock();
            auto now = util::Milliseconds();
            tTask top;
            while (!tasks_.empty()) {
                top = tasks_.top();
                if (top.runat < now + Timer::kLeastDuration) {
                    tasks.push_back(top);
                    tasks_.pop();
                } else {
                    break;
                }
            }
            mtx_.unlock();
            for (auto &&task:tasks) {
                task.run(); /*what if those tasks consume too much time?*/
            }
            now = util::Milliseconds();
            std::unique_lock<std::mutex> ul(mtx_);
            for (auto &&task:tasks) {
                if (task.type == TIMERTASK_REPEATED) {
                    task.runat = now + task.duration;
                    tasks_.push(task);
                }
            }
            if (tasks_.empty()) {
                timer_.reset(Timer::Config{});
                setstate(State::IDLE);
                continue;
            }
            top = tasks_.top();
            ul.unlock();
            if (top.runat < now + Timer::kLeastDuration) {
                post(tTask::Nil);
            } else {
                timer_.reset(Timer::Config{0, 0, 0, unsigned(top.runat - now)});
            }
        }
    }
}