#ifndef MONITOR_FS_H
#define MONITOR_FS_H

#include <string>

#include <fcntl.h>

#include "nocopy.h"

namespace sun {
    namespace util {
        void Close(int &fd);

        bool ValidFd(int fd);

        void RedirectOutput(const char *file);

        class FileHelper {
        public:
            FileHelper() = delete;

            static bool Exist(const char *file);
        };
    }

    enum LockType {
        RECORD = 0,
        UNKNOWN,
    };

    class FileLock : public nocopy {
    public:
        FileLock(std::string file, short type);

        ~FileLock();

        FileLock &setType(short type);

        FileLock &setWhence(short whence);

        FileLock &setStart(off_t start);

        FileLock &setLen(off_t len);

        int lock(bool isblock = false);

        int unlock();

        int test();

        void cleanup();

    protected:
        int Lock(bool isblock = false, bool istest = false);

        int TryOpen();

    private:
        short type_;
        int fd_;
        struct flock fl_;
        std::string file_;
    };
}

#endif //MONITOR_FS_H
