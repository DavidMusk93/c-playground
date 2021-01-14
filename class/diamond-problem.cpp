//
// Created by Steve on 12/31/2020.
// @ref https://www.geeksforgeeks.org/multiple-inheritance-in-c/
//

#include "macro.h"

#include <sstream>
#include <iostream>

#define DTOR(x) ~x(){LOG(#x " dtor");}
#define VIRTUAL virtual

struct Base {
  DTOR(Base);
  unsigned a{0xaabbccdd};
};

struct A : VIRTUAL Base {
  DTOR(A);
};

struct B : VIRTUAL Base {
  DTOR(B);
};

struct C : A, B {
  DTOR(C);
};

static void dump_obj(void *ptr, size_t size) {
  static char buf[8];
  auto op = [](u8 a) {
    sprintf(buf, "%02x", a);
    return buf;
  };
  auto p = reinterpret_cast<u8 *>(ptr);
  std::stringstream ss;
  for (int i = 0; i < size; ++i) {
    if (i) {
      ss << "," << op(p[i]);
    } else {
      ss << op(p[i]);
    }
  }
  std::cout << ss.str() << std::endl;
}

/*
    Base
    /  \
   A    B
   \   /
     C
 */

MAIN() {
#define SIZEOF(x) LOG("sizeof("#x")=%lu",sizeof(x))
#define PTR(x) ((void**)&x)
#define DUMP(x) dump_obj(&x,sizeof(x)-8)
  A a;
  B b;
  C c;
  SIZEOF(Base);
  SIZEOF(A);
  SIZEOF(B);
  SIZEOF(C);
  DUMP(a), DUMP(b), DUMP(c);
}
