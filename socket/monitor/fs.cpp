#include "fs.h"

#include <unistd.h>
#include <fcntl.h>

namespace sun {
    namespace utility {
        void Close(int &fd) {
            if (fd != -1 && ValidFd(fd)) {
                ::close(fd);
            }
            fd = -1;
        }

        bool ValidFd(int fd) {
            return fcntl(fd, F_GETFD) != -1;
        }
    }
}