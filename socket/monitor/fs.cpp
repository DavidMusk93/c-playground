#include "fs.h"

#include <unistd.h>
#include <fcntl.h>

namespace sun {
    namespace util {
        void Close(int &fd) {
            if (fd != -1 && ValidFd(fd)) {
                ::close(fd);
            }
            fd = -1;
        }

        bool ValidFd(int fd) {
            return fcntl(fd, F_GETFD) != -1;
        }

        void RedirectOutput(const char *file) {
            int fd = open(file, O_RDWR | O_CREAT | O_APPEND | O_CLOEXEC);
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
            setbuf(stdout, 0);
            setbuf(stderr, 0);
        }

        bool FileHelper::Exist(const char *file) {
            return ::access(file, F_OK) == 0;
        }
    }

    FileLock::FileLock(std::string file, short type) : type_(type), fd_(-1), fl_{}, file_(std::move(file)) {}

    FileLock::~FileLock() {
        unlock();
        util::Close(fd_);
    }

#define set_type setType
#define set_whence setWhence
#define set_start setStart
#define set_len setLen

#define SETTER(x, __type) \
FileLock&FileLock::set_##x(__type x){\
  fl_.l_##x=x;\
  return *this;\
}

    SETTER(type, short);

    SETTER(whence, short);

    SETTER(start, off_t);

    SETTER(len, off_t);

    int FileLock::lock(bool isblock) {
        if (TryOpen() == -1) {
            return -1;
        }
        return Lock(isblock);
    }

    int FileLock::unlock() {
        if (fd_ == -1 || fl_.l_type == F_UNLCK) {
            return 0;
        }
        fl_.l_type = F_UNLCK;
        return Lock();
    }

    int FileLock::test() {
        if (TryOpen() == -1) {
            return -1;
        }
        return Lock(false, true);
    }

    void FileLock::cleanup() {
        util::Close(fd_);
    }

    int FileLock::Lock(bool isblock, bool istest) {
        static int kMatrix[2/*record(0),ofd(1)*/][3/*nonblock(0),block(1),test(2)*/] = {
                {F_SETLK, F_SETLKW, F_GETLK},
        };
        switch (type_) {
            case RECORD:
                return fcntl(fd_, kMatrix[static_cast<int>(type_)][istest ? 2 : isblock], &fl_);
            default:
                return 0;
        }
    }

    int FileLock::TryOpen() {
        if (fd_ == -1) {
            fd_ = open(file_.c_str(), O_RDWR | O_CREAT | O_NONBLOCK);
        }
        return fd_;
    }
}