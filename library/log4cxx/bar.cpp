//
// Created by Steve on 12/2/2020.
//

#include "bar.h"

using namespace com::foo;
using namespace log4cxx;

LoggerPtr Bar::logger(Logger::getLogger("com.foo.bar"));

void Bar::doIt() {
  LOG4CXX_DEBUG(logger, "Did it again!");
}