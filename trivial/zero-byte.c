//
// Created by Steve on 7/12/2020.
// @ref http://www.spongeliu.com/421.html
//

#define HAS_ZERO_BYTE1(x) ({\
    unsigned char *_p=(unsigned char*)&x;\
    !(_p[0]&&_p[1]&&_p[2]&&_p[3]);\
})

#define HAS_ZERO_BYTE2(x) !((x&0xff)&&(x&0xff00)&&(x&0xff0000)&&(x&0xff000000))

#define HAS_ZERO_BYTE3(x) !!(~(((x&0x7f7f7f7f)+0x7f7f7f7f)|0x7f7f7f7f))

#include "macro.h"

MAIN() {
    unsigned int a = 0x5FF23D6E;
    unsigned int b = 0x5FF2006E;
#define TEST(x) \
LOG("%x",HAS_ZERO_BYTE1(x));\
LOG("%x",HAS_ZERO_BYTE2(x));\
LOG("%x",HAS_ZERO_BYTE3(x));
    TEST(a);
    TEST(b);
}