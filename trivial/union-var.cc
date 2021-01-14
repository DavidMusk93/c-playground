//
// Created by Steve on 12/28/2020.
//

#include "macro.h"

MAIN() {
  union { // cpp auto export anonymous union members?
    u32 x;
    struct {
      u16 a, b;
    } y;
  };
  x = 0x11223344;
  LOG("%#x,%#x", y.a, y.b);
}