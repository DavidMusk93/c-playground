#ifndef MONITOR_PROC_MONITOR_H
#define MONITOR_PROC_MONITOR_H

#include "base.h"
#include "runner.h"

namespace sun {
    namespace io {
        class ProcMonitor : public nocopy, public Runner {
        public:
            using ExecCallback = std::function<void(int pid)>;
            using ExitCallback = std::function<void(int pid, int rc)>;

            ProcMonitor() : nl_sock_(-1) {}

            ~ProcMonitor() override = default;

            bool connect();

            bool subscribe();

        protected:
            void Run() override;

        private:
            int nl_sock_;
            struct {
                bool enable{true};
                int timeout = 60000;
            } config_;
        public:
            ExecCallback on_process_exec;
            ExitCallback on_process_exit;
        };
    }
}

#endif //MONITOR_PROC_MONITOR_H
