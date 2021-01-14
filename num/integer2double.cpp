//
// Created by Steve on 1/7/2021.
//

#include "macro.h"

MAIN() {
  double d;
  int i = -312;
  unsigned u = -1;
  d = i;
  LOG("%u", static_cast<unsigned>(d));
  d = u;
  LOG("%d", static_cast<int>(d));
}