//
// Created by esgyn on 1/15/2021.
//

#ifndef MONITOR_TASK_H
#define MONITOR_TASK_H

#include <thread>

#include <unistd.h>

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

        void cleanup() {
            char c;
            read(fd(), &c, 1);
        }

    private:
        Pipe pipe_;
    };

    class Runner {
    public:

        virtual ~Runner() {
            if (runner_.joinable()) {
                runner_.join();
            }
            terminator_.cleanup();
        }

        void start() {
            runner_ = std::thread([this] { Run(); });
        }

        void stop() {
            terminator_.trigger();
        };

        int quitHandler() const {
            return terminator_.fd();
        }

    protected:
        virtual void Run() = 0;

    private:
        std::thread runner_;
        Terminator terminator_;
    };
}

#endif //MONITOR_TASK_H
