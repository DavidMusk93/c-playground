//
// Created by Steve on 12/2/2020.
//

#include "bar.h"

using namespace com::foo;

// include log4cxx header files.
#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/helpers/exception.h"

using namespace log4cxx;
using namespace log4cxx::helpers;
// Define a static logger variable so that it references the
// Logger instance named "MyApp".
LoggerPtr logger(Logger::getLogger("MyApp"));

int main(int argc, char **argv) {
  int result = EXIT_SUCCESS;
  try {
    if (argc > 1) {
      // BasicConfigurator replaced with PropertyConfigurator.
      PropertyConfigurator::configure(argv[1]);
    } else {
      BasicConfigurator::configure();
    }

    LOG4CXX_INFO(logger, "Entering application.");
    Bar bar;
    bar.doIt();
    LOG4CXX_INFO(logger, "Exiting application.");
  }
  catch (Exception &) {
    result = EXIT_FAILURE;
  }

  return result;
}