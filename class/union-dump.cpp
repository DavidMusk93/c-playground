//
// Created by Steve on 1/5/2021.
//

#include <iostream>

#include "macro.h"

struct A {
  enum class Type : char {
    S8,
    S32,
    F32,
    F64,
    NONE,
  };
  A(Type t) : u_(), t_(t) {}

  A(Type t, void *ptr) : t_(t) {
    switch (t_) {
      case Type::S8:u_.c = *reinterpret_cast<char *>(ptr);
        break;
      case Type::S32:u_.i = *reinterpret_cast<int *>(ptr);;
        break;
      case Type::F32:u_.f = *reinterpret_cast<float *>(ptr);;
        break;
      case Type::F64:u_.d = *reinterpret_cast<double *>(ptr);;
        break;
      default:break;
    }
  }

  ~A() = default;

  char &getChar() {
    return u_.c;
  }

  int &getInt() {
    return u_.i;
  }

  float &getFloat() {
    return u_.f;
  }

  double &getDouble() {
    return u_.d;
  }

  Type getType() const {
    return t_;
  }

  A &&move() {
    return static_cast<A &&>(*this);
  }

  friend std::ostream &operator<<(std::ostream &os, A &&a);

 private:
  union {
    char c;
    int i;
    float f;
    double d;
  } u_;
  Type t_;
};

std::ostream &operator<<(std::ostream &os, A &&a) {
  using Type = A::Type;
  switch (a.getType()) {
    case Type::S8:os << a.getChar();
      break;
    case Type::S32:os << a.getInt();
      break;
    case Type::F32:os << a.getFloat();
      break;
    case Type::F64:os << a.getDouble();
      break;
    default:break;
  }
  return os;
}

MAIN() {
  A a1(A::Type::S32);
  a1.getInt() = 312;
  A a2(A::Type::F32);
  a2.getFloat() = 3.12;
  std::cout << a1.move() << "|" << a2.move() << std::endl;
  int a = 312;
  float b = 3.12;
  std::cout << A(A::Type::S32, &a) << "|" << A(A::Type::F32, &b) << std::endl;
}