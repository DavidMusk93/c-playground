//
// Created by Steve on 12/17/2020.
//

#include "macro.h"

class Base {
 public:
  char c;
  virtual ~Base() {}
};

class A : public Base {
 public:
  float x;
};

class B : public Base {
 public:
  int x;
};

MAIN() {
  A a;
  B b1, b2;
  LOG("%zu,%zu,%zu", sizeof(Base), sizeof(A), sizeof(B));
  LOG("%p,%p", *(void **) &b1, *(void **) &b2);
}