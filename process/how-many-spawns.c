//
// Created by Steve on 7/12/2020.
//

#include <unistd.h>

#include "macro.h"

MAIN() {
    fork();
    fork() && fork() || fork();
    fork();
    LOG("done");
}