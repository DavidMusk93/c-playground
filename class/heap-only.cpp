//
// Created by Steve on 12/31/2020.
// @ref http://purecpp.org/detail?id=2209
//

#include <memory>

class dummy {
 public:
  struct token {
   private:
    token() = default;
    friend dummy;
  };

  static std::shared_ptr<dummy> create() {
    return std::make_shared<dummy>(token{});
  }

  dummy(token) : dummy() {}

  int &GetInt() {
    return a_;
  }

 private:
  dummy() = default;
  int a_;
};

#include "macro.h"

MAIN() {
  char buf[4]{};
  reinterpret_cast<dummy *>(buf)->GetInt() = 0x11223344;
  LOG("%#x,%#x,%#x,%#x", buf[0], buf[1], buf[2], buf[3]);
}