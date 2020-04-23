//
// Created by Steve on 4/21/2020.
//

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#define LOG(fmt, ...) printf(fmt"\n",##__VA_ARGS__)

int main() {
    struct timeval tv = {};
    struct tm tm = {};
    struct timezone tz = {};
    gettimeofday(&tv, &tz);
    // gmtime_r(&tv.tv_sec, &tm);
    localtime_r(&tv.tv_sec, &tm);
    LOG("%d %d:%d:%d", tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    return 0;
}