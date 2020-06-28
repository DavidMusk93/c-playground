//
// Created by Steve on 6/27/2020.
//

#include <stdio.h>

int nGlobalVar = 0;

int foo(int a, int b) {
    printf("%s is called, a = %d, b = %d\n", __func__, a, b);
    return (a + b);
}

int main() {
    int n;
    n = 1;
    n++;
    n--;

    nGlobalVar += 100;
    nGlobalVar -= 12;

    printf("n = %d, nGlobalVar = %d\n", n, nGlobalVar);

    n = foo(1, 2);
    printf("n = %d\n", n);

    return 0;
}