//
// Created by Steve on 12/28/2020.
//

#include "macro.h"

MAIN() {
  auto a = new int;
  int *b = nullptr;
  void *c = new float;
  *a = 1;
  delete a;
  delete b; // its ok to delete nullptr
  delete (float *) c; // pointer type is required (to call specific dtor?)
}