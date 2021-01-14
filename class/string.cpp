//
// Created by Steve on 7/13/2020.
//

#include <string>
#include <type_traits>

using namespace std;

#include "macro.h"

struct A {
  int a;
  float b;
  unsigned c;
  A(int a, float b, unsigned c) : a(a), b(b), c(c) {}
  double foo() {
    return double((unsigned) a ^ c) * b;
  }
};

#define BOOL_STR(x) (x?"true":"false")

MAIN() {
  string empty;
  LOG("sizeof empty string:%zu", sizeof(empty));
  string s{"hello"};
  decltype(s.begin()) it; //simplify declaration
  decltype(declval<A>().foo()) d; //'call' member function without instance
  LOG("d is float point: %s", BOOL_STR(is_floating_point_v<decltype(d)>));
  const auto &t = d;
//    using V=const double&;
  using T = decay_t<decltype(t)>; //remove const & reference (cpp11+ is required)
  LOG("const or reference: %d", is_const_v<T> || is_reference_v<T>);
  it = s.begin() + 3;
  auto next = s.erase(it);
  LOG("erase returned: %c", next != s.end() ? *next : '@');
  LOG("%s", s.c_str());
}