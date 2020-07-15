//
// Created by Steve on 7/14/2020.
// @ref:
//   *https://stackoverflow.com/questions/2535989/what-are-the-calling-conventions-for-unix-linux-system-calls-on-i386-and-x86-6
//   *https://stackoverflow.com/questions/10385784/how-to-get-a-char-with-ptrace
//   *https://www.linuxjournal.com/article/6100
//

#include <string.h>
#include <ctype.h>

#include "common.h"

#define LONG_MAGIC 0x7f7f7f7f7f7f7f7f
#define LONG_HAS_ZERO_BYTE(x) (~(((x&LONG_MAGIC)+LONG_MAGIC)|LONG_MAGIC)!=0)

#define REG_TYPE unsigned long long
#define REG_FMT "%llu"
#define LONG_SIZE ((int)sizeof(long))

#define PEEK_REG(pid, reg) \
ptrace(PTRACE_PEEKUSER,pid,USER_REGS_OFF(reg),0)

#define GETARGS_CUMBERSOME 1

static const char *dump_string(pid_t child, REG_TYPE addr) {
    static char data[4096]; /* static variable most likely to be harmful */
    int nr = 0;
    unsigned long v;
    while (1) {
        v = ptrace(PTRACE_PEEKDATA, child, addr + nr, 0);
        if (errno) {
            data[nr] = 0;
            break;
        }
        memcpy(data + nr, &v, sizeof v);
        if (/*memchr(&v,0,sizeof v)*/LONG_HAS_ZERO_BYTE(v)) {
            break;
        }
        nr += sizeof v;
    }
    return data;
}

static void reverse(char *str, int jump_nl) {
    int i = 0, j = (int) strlen(str) - 1;
    char t;
    if (jump_nl) {
        for (; isspace(str[j]); --j);
    }
    for (; i < j; ++i, --j) {
        t = str[i];
        str[i] = str[j];
        str[j] = t;
    }
}

void peek_data(pid_t child, REG_TYPE addr, char *str, int len) {
    char *p;
    int i, j;
    long data;
    for (i = 0, j = len / LONG_SIZE, p = str; i < j; ++i, p += LONG_SIZE) {
        data = ptrace(PTRACE_PEEKDATA, child, addr + i * LONG_SIZE, 0);
        memcpy(p, &data, LONG_SIZE);
    }
    j = len % LONG_SIZE;
    if (j) {
        data = ptrace(PTRACE_PEEKDATA, child, addr + i * LONG_SIZE, 0);
        memcpy(p, &data, j);
    }
    str[len] = 0;
}

void poke_data(pid_t child, REG_TYPE addr, const char *str, int len) {
    const char *p;
    int i, j;
    long data;
    for (i = 0, j = len / LONG_SIZE, p = str; i < j; ++i, p += LONG_SIZE) {
        memcpy(&data, p, LONG_SIZE);
        ptrace(PTRACE_POKEDATA, child, addr + i * LONG_SIZE, data/*error-prone*/);
    }
    j = len % LONG_SIZE;
    if (j) {
        memcpy(&data, p, j);
        ptrace(PTRACE_POKEDATA, child, addr + i * LONG_SIZE, data);
    }
}

MAIN() {
    pid_t child;
    long orig_rax, rax;
    REG_TYPE params[3];
    int status;
    int insyscall = 0;
    child = fork();
    if (child == 0) {
        ptrace(PTRACE_TRACEME, 0, 0, 0);
        execl("/bin/ls", "ls", "-l", NULL);
    } else {
        while (1) {
            wait(&status);
            if (WIFEXITED(status)) {
                break;
            }
            orig_rax = PEEK_REG(child, orig_rax);
            LOG("SYSCALL number: %ld", orig_rax);
            if (orig_rax == SYS_write) {
                if (insyscall == 0) {
                    insyscall = 1;
#if GETARGS_CUMBERSOME
                    struct user_regs_struct regs;
                    ptrace(PTRACE_GETREGS, child, 0, &regs);
                    params[0] = regs.rdi, params[1] = regs.rsi, params[2] = regs.rdx;
//                    long ins=ptrace(PTRACE_PEEKTEXT,child,regs.rip,0);
//                    LOG("@TEXT "REG_FMT" instruction executed: %lx",regs.rip,ins);
#else
                    params[0]=PEEK_REG(child,rdi);
                    params[1]=PEEK_REG(child,rsi);
                    params[2]=PEEK_REG(child,rdx);
#endif
//                    printf("@BUFFER '%s'",dump_string(child,params[1]));
                    char *p = malloc(params[2] + 1);
                    peek_data(child, params[1], p, (int) params[2]);
                    reverse(p, 1);
//                    printf("@BUFFER '%s'",p);
                    poke_data(child, params[1], p, (int) params[2]);
                    free(p);
                    LOG("WRITE called with "REG_FMT","REG_FMT","REG_FMT, params[0], params[1], params[2]);
                } else {
                    rax = PEEK_REG(child, rax);
                    LOG("WRITE returned with %ld", rax);
                    insyscall = 0;
                }
            }
            ptrace(PTRACE_SYSCALL, child, 0, 0);
        }
    }
    return 0;
}
