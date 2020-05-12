//
// Created by david on 5/11/20.
//

#ifndef C4FUN_NOW_H
#define C4FUN_NOW_H

#include <time.h>
#include <sys/time.h>

struct timeval *now(struct timeval *ptr);

typedef const char *(FN_format)(const struct tm *ptr, int ms, char buf[64]);

const char *default_formatter(const struct tm *ptr, int ms, char buf[64]);

const char *time_format(const struct timeval *tv, FN_format *fn);

#define DEFAULT_TIME_FORMAT(x) time_format(x,&default_formatter)

#endif //C4FUN_NOW_H
