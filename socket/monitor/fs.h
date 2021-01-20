#ifndef MONITOR_FS_H
#define MONITOR_FS_H

namespace sun {
    namespace utility {
        void Close(int &fd);

        bool ValidFd(int fd);
    }
}

#endif //MONITOR_FS_H
