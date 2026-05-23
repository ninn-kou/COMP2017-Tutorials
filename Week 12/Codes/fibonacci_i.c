// 2026 S1C COMP2017 Week 12 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Fibonacci Numbers - I
// By using the pure recursion without optimizations.


#include <stdio.h>
#include <stdlib.h>

long fib(int n) {
    if (n <= 1) {
        return n;
    }

    return fib(n - 1) + fib(n - 2);
}

int main(int argc, char *argv[]) {
    int n = 42;

    if (argc == 2) {
        n = atoi(argv[1]);
    }

    if (n < 0) {
        fprintf(stderr, "n must be non-negative\n");
        return 1;
    }

    printf("%ld\n", fib(n));

    return 0;
}
