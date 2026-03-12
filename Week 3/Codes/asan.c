// 2026 S1C COMP2017 Week 3 Tutorial B Extension Codes
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Runtime Instrumentation, Try:
//     $ gcc -std=c23 asan.c -o plain
//     $ gcc -std=c23 -fsanitize=address -g -O0 asan.c -o asan
//     $ ./plain
//     $ ./asan


#include <stdlib.h>

int main(void) {
    int *p = malloc(2 * sizeof(int));
    p[2] = 42;   /* out of bounds */
    free(p);
    return 0;
}
