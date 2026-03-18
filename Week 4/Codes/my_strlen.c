// 2026 S1C COMP2017 Week 4 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Simple "my_strlen()"" function.


#include <stdio.h>

int my_strlen(const char* s) {
    int i = 0;

    while (s[i] != '\0') {
        i++;
    }

    return i;
}

int main(void) {
    printf("%d\n", my_strlen(""));               // should output 0
    printf("%d\n", my_strlen("123"));            // should output 3
    printf("%d\n", my_strlen("abc\n"));          // should output 4
    printf("%d\n", my_strlen("lorem\0ipsum\n")); // should output 5
    printf("%d\n", my_strlen("lorem ipsum\n"));  // should output 12

    return 0;
}
