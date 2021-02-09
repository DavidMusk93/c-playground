#ifndef MONITOR_COORDINATOR_H
#define MONITOR_COORDINATOR_H

#include <list>

#include "sock.h"
#include "file_watcher.h"
#include "common.h"
#include "loop.h"

namespace sun {
    class Coordinator : public Loop {
    public:
        struct WorkerMeta {
            int pid{0};
            int handler{-1};
        };

        using Iterator = std::list<WorkerMeta>::iterator;

        enum class WorkerType : char {
            BUSY,
            IDLE,
        };

        Coordinator();

        ~Coordinator();

        void loop() override;

        std::list<WorkerMeta> &workers(WorkerType type) {
            static std::list<WorkerMeta> empty;
            if (type == WorkerType::BUSY) {
                return busy_workers_;
            } else if (type == WorkerType::IDLE) {
                return idle_workers_;
            } else {
                return empty;
            }
        }

    private:
        char ipc_[64];
        io::FileWatcher fw_;
        Defer cleanup_;
        std::list<WorkerMeta> busy_workers_;
        std::list<WorkerMeta> idle_workers_;
        struct {
            short port{SERVER_PORT};
        } config_;
    };
}

#endif //MONITOR_COORDINATOR_H
