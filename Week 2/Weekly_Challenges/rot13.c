// 2026 S1C COMP2017 Week 1 Task 4.
// Tutor: Hao Ren (hao.ren@sydney.edu.au)


#include <stdio.h>

int main(void) {
    int c;

    while ((c = getchar()) != EOF) {
        if (c >= 'a' && c <= 'z') {
            c = 'a' + (c - 'a' + 13) % 26;
        } else if (c >= 'A' && c <= 'Z') {
            c = 'A' + (c - 'A' + 13) % 26;
        }
        putchar(c);
    }

    return 0;
}
