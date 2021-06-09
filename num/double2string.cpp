#include "macro.h"

#include "double-conversion/double-to-string.h"

MAIN() {
    double a = 1234567890.;
    double b = 123456789.123456789;
    using namespace double_conversion;
    auto mode = DoubleToStringConverter::PRECISION;
    char buf[1024];
    bool sign;
    int len;
    int point;
    DoubleToStringConverter::DoubleToAscii(a, mode, 31, buf, sizeof buf, &sign, &len, &point);
    LOG("%s,%d,%d,%d", buf, sign, len, point);
    DoubleToStringConverter::DoubleToAscii(b, mode, 31, buf, sizeof buf, &sign, &len, &point);
    LOG("%s,%d,%d,%d", buf, sign, len, point);
}