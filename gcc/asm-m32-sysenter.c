//
// Created by Steve on 7/15/2020.
//

#include <stdlib.h>
#include <elf.h>

#include "macro.h"

#define MAIN_EX3(argc, argv, envp) \
int main(int argc,char *argv[],char *envp[])

MAIN_EX3(argc, argv, envp) {
    unsigned int syscall_nr = 1;
    int exit_status = 42;
    Elf32_auxv_t *auxv;
    while (*envp++); //auxiliary vectors are located after the end of the environment
    for (auxv = (Elf32_auxv_t *) envp; auxv->a_type != AT_NULL; ++auxv) {
        if (auxv->a_type == AT_SYSINFO) { //find __kernel_vsyscall
            break;
        }
    }
    asm("movl %0,%%eax\n"
        "movl %1,%%ebx\n"
        "call *%2\n"
    : /* output parameters */
    : /* input parameters mapped to %0, %1 respectively */
    "m"(syscall_nr), "m"(exit_status), "m"(auxv->a_un.a_val)
    : /* registers that we are clobbering */
    "eax", "ebx");
}