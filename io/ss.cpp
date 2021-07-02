#include "macro.h"

#include <sstream>
#include <iostream>

MAIN() {
    std::stringstream ss;
    std::string s = "sun";
    ss << s;
    ss << char(0);
    auto s2 = ss.str();
    std::cout << s2.size() << std::endl;
    return 0;
}