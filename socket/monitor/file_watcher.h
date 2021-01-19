#ifndef MONITOR_FILE_WATCHER_H
#define MONITOR_FILE_WATCHER_H

#include <string>
#include <vector>

#include <sys/inotify.h>
#include <fcntl.h>
#include <unordered_map>

#include "base.h"
#include "runner.h"

namespace sun {
    namespace io {
        class FileWatcher : public nocopy, public Runner {
        public:
            using Event = struct inotify_event;
            using Callback = std::function<void(Event *)>;

            enum class EventType : char {
                CREATE,
                DELETE,
            };

            FileWatcher();

            ~FileWatcher() override;

            bool watchPath(std::string &&path, unsigned mask = IN_CREATE | IN_DELETE);

            FileWatcher &registerCallback(EventType type, Callback callback);

        protected:
            void Run() override;

        private:
            std::vector<Defer> cleanup_;
            std::unordered_map<int, std::string> watched_paths_;
            int inotify_handler_;
            Callback on_file_create_;
            Callback on_file_delete_;
            struct Config {
                int timeout{60000};
            } config_;
        };
    }
}

#endif //MONITOR_FILE_WATCHER_H
