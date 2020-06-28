//
// Created by Steve on 6/21/2020.
//

#include "macro.h"

MAIN() {
//    int a, b;
//    char str[20];
//    scanf("%d %d", &a, &b);
//    LOG("%d %d", a, b);
//    /* On success, the function returns the number of items of the argument list successfully read.
//     * For string, a terminating null character is automatically added at the end of the stored sequence. */
//    int n = scanf("%5s", str);
//    LOG("%d %s", n, str);
    char c = 0x91;
    LOG("char cast to int %d <-> %u <-> %d", (int) c, (unsigned) c, (unsigned char) c);
    return 0;
}