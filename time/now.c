//
// Created by Steve on 4/21/2020.
//

#include "now.h"

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#define LOG(fmt, ...) printf(fmt"\n",##__VA_ARGS__)

struct timeval *now(struct timeval *ptr) {
    static __thread struct timeval tv;
    struct timeval *p = ptr ? ptr : &tv;
    gettimeofday(p, 0);
    return p;
}

const char *default_formatter(const struct tm *ptr, int ms, char buf[64]) {
    sprintf(buf, "%4d/%02d/%02d-%02d:%02d:%02d.%03d",
            ptr->tm_year + 1900, ptr->tm_mon + 1, ptr->tm_mday,
            ptr->tm_hour, ptr->tm_min, ptr->tm_sec,
            ms);
    return buf;
}

const char *time_format(const struct timeval *tv, FN_format *fn) {
    static __thread char buf[64];
    struct tm tm;
    localtime_r(&tv->tv_sec, &tm);
    return fn(&tm, (int) tv->tv_usec / 1000, buf);
}

#ifdef TEST_TIME
int main() {
    struct timeval tv;
    LOG("%s", DEFAULT_TIME_FORMAT(now(&tv)));
    return 0;
}
#endif