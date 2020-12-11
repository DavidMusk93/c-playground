//
// Created by Steve on 12/2/2020.
//

#ifndef LOG4CXXTEST__BAR_H_
#define LOG4CXXTEST__BAR_H_

#include <log4cxx/logger.h>

namespace com {
namespace foo {
class Bar {
  static log4cxx::LoggerPtr logger;

 public:
  void doIt();
};
}
}

#endif //LOG4CXXTEST__BAR_H_
