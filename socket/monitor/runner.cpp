#include "runner.h"

#include <vector>

#include <sys/poll.h>

namespace sun {
    void *Task::kClosureTag = (void *) -1;

    void Task::run() const {
#define CALL(fn, ...) if(fn){fn(__VA_ARGS__);}
        CALL(hook.oncall, arg);
        void *p{};
        try {
            if (arg == kClosureTag) {
                closure();
            } else {
                p = fn(arg);
            }
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

    int tRunner::post(const tTask &task) {
        if (task.fn || task.closure) {
            task.id = __sync_add_and_fetch(&id_, 1);
            {
                LOCKGUARD(mark_mtx_);
                marks_.insert(task.id);
            }
            {
                LOCKGUARD(mtx_);
                tasks_.push(task);
            }
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
            return task.id;
        }
        notifier_.notify(Notifier::kNotifyOne);
        return -1;
    }

    void tRunner::cancel(int id) {
        if (id > 0 && id < id_/*this constraint may be wrong*/) {
            LOCKGUARD(mark_mtx_);
            marks_.erase(id);
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
            bool alive;
            while (!tasks_.empty()) {
                top = tasks_.top();
                if (top.runat < now + Timer::kLeastDuration) {
                    {
                        LOCKGUARD(mark_mtx_);
                        alive = marks_.count(top.id);
                    }
                    if (alive) {
                        tasks.push_back(top);
                    } else { /*the task is canceled, clear it*/
                        if (top.dispose) {
                            top.dispose(top.arg);
                        }
                    }
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
                if ((task.type & tTask::kTypeMask) == TIMERTASK_REPEATED) {
                    task.runat = now + task.duration;
                    tasks_.push(task);
                } else { /*drop once task*/
                    if (task.dispose) {
                        task.dispose(task.arg);
                    }
                    LOCKGUARD(mark_mtx_);
                    marks_.erase(task.id);
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