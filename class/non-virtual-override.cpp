//
// Created by Steve on 12/31/2020.
//

#include "macro.h"

#define DTOR(x) ~x(){LOG(#x " dtor");}

struct A {
  DTOR(A);
};

struct B : A {
  DTOR(B);
};

MAIN() {
  B derived;
}