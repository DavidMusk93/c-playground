#include "file_watcher.h"
#include "test.h"

static void ProcCreateFileHandle(struct inotify_event *event) {
    LOGINFO("@%s %s %#x", __func__, event->name, event->mask);
}

static void ProcDeleteFileHandle(struct inotify_event *event) {
    LOGINFO("@%s %s %#x", __func__, event->name, event->mask);
}

#if 0
int main() {
    sun::io::FileWatcher fw;
    fw.registerCallback(sun::io::FileWatcher::EventType::CREATE, &ProcCreateFileHandle)
            .registerCallback(sun::io::FileWatcher::EventType::DELETE, &ProcDeleteFileHandle)
            .watchPath("/tmp/");
    fw.start();
    SIGBLOCK();
    fw.stop();
}
#endif