#include <string>

#include "macro.h"

std::string a = "hi,there!";

MAIN() {
    LOG("%s,%p,%p", a.c_str(), &a, &a[0]);
}