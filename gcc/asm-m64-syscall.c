//
// Created by Steve on 7/15/2020.
// @ref
//   *http://arthurchiao.art/blog/system-call-definitive-guide-zh/
//

#define USE_WRAPPER 0 /* using syscall wrapper */

#if USE_WRAPPER
#include <unistd.h>
#endif

#include <sys/syscall.h>

#include "macro.h"

MAIN() {
    long syscall_nr = SYS_exit;
    long exit_status = 42;
#if USE_WRAPPER
    syscall(syscall_nr,exit_status);
#else
    asm("movq %0,%%rax\n"
        "movq %1,%%rdi\n"
        "syscall"
    :
    :
    "m"(syscall_nr), "m"(exit_status)
    :
    "rax", "rdi");
#endif
}