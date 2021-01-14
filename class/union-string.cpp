//
// Created by Steve on 1/5/2021.
// @ref https://stackoverflow.com/questions/3521914/why-compiler-doesnt-allow-stdstring-inside-union
//

#include <string>

#include "macro.h"

union UF {
  UF():s{}{}
  std::string s;
  float f;
  double d;
};

MAIN() {
  UF u1, u2;
  u1.f = 3.12;
  std::string s = u1.s;
  u2.s = s;
  LOG("%g", u2.f);
}