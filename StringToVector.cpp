//
// Created by Steve on 4/21/2020.
//

#include <vector>

#include <string.h>
#include <stdio.h>

#define SPACE ' '

static std::vector<char *> stringToVector(char *s) {
    std::vector<char *> res;
    res.reserve(16);
    while (s) {
        res.push_back(s);
        s = strchr(s, SPACE);
        if (s) {
            *s++ = 0;
        }
    }
    res.push_back(0);
    return res;
}

#undef SPACE
#if TEST_STRINGTOVECTOR
int main() {
    char s[] = "a b c";
    auto res = stringToVector(s);
    char **argv = res.data();
    while (*argv) {
        puts(*argv++);
    }
    return 0;
}
#endif