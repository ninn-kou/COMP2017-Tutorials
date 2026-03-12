// 2026 S1C COMP2017 Week 3 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Convert a string type number to integer.


#include <stdio.h>

int atoi(const char s[]) {
    int i = 0;
    int sign = 1;
    int n = 0;

    /* Skip leading whitespace */
    while (s[i] == ' ' || s[i] == '\t' || s[i] == '\n') {
        i++;
    }

    /* Check sign */
    if (s[i] == '-') {
        sign = -1;
        i++;
    } else if (s[i] == '+') {
        i++;
    }

    /* Read digits and build the number */
    while (s[i] >= '0' && s[i] <= '9') {
        n = n * 10 + (s[i] - '0');
        i++;
    }

    return sign * n;
}

int main(void) {
    printf("%d\n", atoi("0"));        // 0
    printf("%d\n", atoi("0123"));     // 123
    printf("%d\n", atoi("1234"));     // 1234
    printf("%d\n", atoi(""));         // 0
    printf("%d\n", atoi("-1234"));    // -1234

    /* More test cases */
    printf("%d\n", atoi("+56"));      // 56
    printf("%d\n", atoi("00042"));    // 42
    printf("%d\n", atoi("12abc"));    // 12
    printf("%d\n", atoi("abc"));      // 0
    printf("%d\n", atoi("   99"));    // 99
    printf("%d\n", atoi("-0"));       // 0

    return 0;
}
