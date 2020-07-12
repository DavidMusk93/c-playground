//
// Created by Steve on 7/12/2020.
//

#include <unistd.h>

#include "macro.h"

MAIN() {
    for (int i = 0; i < 5; ++i) {
        fork();
        LOG("%d", getpid());
        fflush(stdout);
    }
    pause();
}