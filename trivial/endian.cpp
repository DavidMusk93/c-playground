//
// Created by Steve on 1/7/2021.
// @ref https://stackoverflow.com/questions/4239993/determining-endianness-at-compile-time
//

#include "macro.h"

//static union {
//  int __x = 0x11223344;
//  char __p[4];
//};

static constexpr int _TMP = 0x11223344;
//static constexpr char __first_char = static_cast<char>(_TMP);
//#if __first_char == 0x44
#if 'ABCD' == 0x44434241
#define LE 1
#else
#define LE 0
#endif

MAIN() {
  LOG("%d", LE);
  auto p = reinterpret_cast<const char *>(&_TMP);
  LOG("%#x,%#x,%#x,%#x", p[0], p[1], p[2], p[3]);
#define CAST(x, i) static_cast<char>(x>>i)
  LOG("%#x,%#x,%#x,%#x", CAST(_TMP, 0), CAST(_TMP, 8), CAST(_TMP, 16), CAST(_TMP, 24));
}