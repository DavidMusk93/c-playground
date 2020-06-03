#include "macro.h"

#include <stdio.h>
#include <vector>

const char *print_int(const void *ptr) {
    static __thread char buf[16];
    sprintf(buf, "'%d'", *(int *) ptr);
    return buf;
}

template<class T>
void dump_vector(const T &vec, const char *(*converter)(const void *)) {
    for (int i = 0, n = vec.size(); i < n; ++i) {
        puts(converter((void *) &vec[i]));
    }
}

MAIN() {
    std::vector<int> data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<int> v1(data.begin() + 1 /* include */, data.begin() + 5 /* exclude */);
    dump_vector(v1, &print_int);
//    std::vector<int> v2(data.begin() + 8, data.begin() + 4);
//    dump_vector(v2, &print_int);
}