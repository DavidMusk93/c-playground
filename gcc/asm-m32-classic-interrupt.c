//
// Created by Steve on 7/15/2020.
//

#include "macro.h"

MAIN() {
    unsigned int syscall_nr = 1;
    int exit_status = 42;
    asm("movl %0,%%eax\n"
        "movl %1,%%ebx\n"
        "int $0x80"
    :
    :
    "m"(syscall_nr), "m"(exit_status)
    :
    "eax", "ebx");
}