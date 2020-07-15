//
// Created by Steve on 7/14/2020.
//

#include <unistd.h>

#include "macro.h"

MAIN() {
    for (int i = 0; i < 10; ++i) {
        LOG("my counter: %d", i);
        sleep(2);
    }
    return 0;
}