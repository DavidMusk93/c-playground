#include <unistd.h>
#include <poll.h>

#include "file_watcher.h"

namespace sun {
    namespace io {
        FileWatcher::FileWatcher() {
            inotify_handler_ = inotify_init1(O_NONBLOCK);
        }

        FileWatcher::~FileWatcher() {
            close(inotify_handler_);
        }

        bool FileWatcher::watchPath(std::string &&path, unsigned mask) {
            int watch_fd = inotify_add_watch(inotify_handler_, path.c_str(), mask);
            if (watch_fd == -1) {
                LOGERROR("Fail to watch %s", path.c_str());
                return false;
            }
            watched_paths_[watch_fd] = std::move(path);
            cleanup_.emplace_back([this, watch_fd] { inotify_rm_watch(inotify_handler_, watch_fd); });
            return true;
        }

        FileWatcher &FileWatcher::registerCallback(EventType type, Callback callback) {
            if (type == EventType::CREATE) {
                on_file_create_.swap(callback);
            } else if (type == EventType::DELETE) {
                on_file_delete_.swap(callback);
            }
            return *this;
        }

        void FileWatcher::Run() {
            int nfds, nr;
            char buf[4096];
            struct pollfd pfds[2]{};
            pfds[0] = {.fd=notifier(), .events=POLLIN,};
            pfds[1] = {.fd=inotify_handler_, .events=POLLIN,};
            for (;;) {
                POLL(nfds, poll, pfds, 2, config_.timeout);
                if (pfds[0].revents & POLLIN) {
                    break;
                }
                if (pfds[1].revents & POLLIN) {
                    nr = read(pfds[1].fd, buf, sizeof(buf));
                    if (nr == -1) {
                        if (errno != EAGAIN) {
                            LOGERROR("Read failure: %s", ERRNOSTR);
                            break;
                        }
                        continue;
                    }
                    for (auto p = buf; p < buf + nr;) {
                        auto event = reinterpret_cast<Event *>(p);
                        if (event->mask & IN_CREATE && on_file_create_) {
                            on_file_create_(event);
                        } else if (event->mask & IN_DELETE && on_file_delete_) {
                            on_file_delete_(event);
                        }
                        p += sizeof(Event) + event->len;
                    }
                }
            }
        }
    }
}