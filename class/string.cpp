//
// Created by Steve on 7/13/2020.
//

#include <string>

using namespace std;

#include "macro.h"

MAIN() {
    string s{"hello"};
    LOG("%s", s.substr(s.size()).c_str());
}