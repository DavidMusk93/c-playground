//
// Created by Steve on 1/5/2021.
//

#include <stdlib.h>

#include "macro.h"

MAIN() {
  double d = 1024.312456987;
  int dec, sign;
  dec = sign = 0;
  const char *str = fcvt(d, 5, &dec, &sign);
  LOG("%s,%d,%d", str, dec, sign);
}