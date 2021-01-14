//
// Created by Steve on 12/30/2020.
//

#include <vector>

#include "macro.h"

struct A {
  A(std::vector<int> data) : data_(std::move(data)) {}
  A(A &&a) {
    data_.swap(a.data_);
  }
  A &&move() {
    return static_cast<A &&>(*this);
  }
  size_t length() const {
    return data_.size();
  }
 private:
  std::vector<int> data_;
};

MAIN() {
  std::vector<int> data{1, 2, 3};
  A a1(std::move(data));
  A a2(a1.move());
  LOG("%zu,%zu,%zu", data.size(), a1.length(), a2.length());
}