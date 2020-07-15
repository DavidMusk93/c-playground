//
// Created by Steve on 7/14/2020.
//

#ifndef C4FUN_COMMON_H
#define C4FUN_COMMON_H

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>
#include <assert.h>
#include <sys/syscall.h>
#include <errno.h>

#include "macro.h"

#define USER_REGS_OFF(x) (&((struct user_regs_struct*)0)->x)

#endif //C4FUN_COMMON_H
