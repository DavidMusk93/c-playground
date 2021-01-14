//
// Created by Steve on 1/2/2021.
//

#include <string>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include "macro.h"

class FileLock {
 public:
  enum class Type : char {
    RECORD,
    OFD,
    UNKNOWN,
  };
  FileLock(std::string file, Type type) : type_(type), fd_(-1), fl_{}, file_(std::move(file)) {}

  ~FileLock() {
    if (fd_ == -1) {
      return;
    }
    unlock();
    close(fd_);
  }

#define set_type setType
#define set_whence setWhence
#define set_start setStart
#define set_len setLen

#define SETTER(x, __type) \
FileLock&set_##x(__type x){\
  fl_.l_##x=x;\
  return *this;\
}

  SETTER(type, short);
  SETTER(whence, short);
  SETTER(start, off_t);
  SETTER(len, off_t);

  int lock() {
    if (fd_ == -1) {
      fd_ = open(file_.c_str(), O_RDWR | O_NONBLOCK);
      if (fd_ == -1) {
        return fd_;
      }
    }
    return Lock();
  }

  int unlock() {
    if (fd_ == -1 || fl_.l_type == F_UNLCK) {
      return 0;
    }
    fl_.l_type = F_UNLCK;
    return Lock();
  }

 protected:
  int Lock() {
    switch (type_) {
      case Type::RECORD:return fcntl(fd_, F_SETLK, &fl_);
      case Type::OFD:return fcntl(fd_, F_OFD_SETLK, &fl_);
      default:return 0;
    }
  }

 private:
  Type type_;
  int fd_;
  struct flock fl_;
  std::string file_;
};

MAIN_EX(argc, argv) {
  do {
    if (argc == 1) {
      break;
    }
    FileLock fl(argv[1], FileLock::Type::OFD);
    auto ret = fl.setType(F_WRLCK).setWhence(SEEK_SET).setStart(10).setLen(20).lock();
    if (ret == -1) {
      LOG("fail to lock %s: %m", argv[1]);
      break;
    }
    sigset_t sigset{};
    int sig{};
    sigaddset(&sigset, SIGINT);
    return sigwait(&sigset, &sig);
  } while (false);
  return 1;
}