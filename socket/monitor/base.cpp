//
// Created by esgyn on 1/15/2021.
//

#include "base.h"

#include <unistd.h>

namespace sun {
    namespace utility {
        int getpid() {
            static thread_local int pid = ::getpid();
            return pid;
        }
    }
}