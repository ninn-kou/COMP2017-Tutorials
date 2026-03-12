// 2026 S1C COMP2017 Week 3 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Reverse a string char-by-char.


#include <stdio.h>

void reverse(char s[]) {
    int len = 0;
    int i, j;
    char temp;

    /* Find the length of the string */
    while (s[len] != '\0') {
        len++;
    }

    /* If there is a newline at the end, do not reverse it */
    if (len > 0 && s[len - 1] == '\n') {
        len--;
    }

    /* Reverse the characters from s[0] to s[len - 1] */
    i = 0;
    j = len - 1;

    while (i < j) {
        temp = s[i];
        s[i] = s[j];
        s[j] = temp;
        i++;
        j--;
    }
}

int main(void) {
    char line[102];   /* 100 chars + '\n' + '\0' */

    while (fgets(line, sizeof line, stdin) != NULL) {
        reverse(line);
        printf("%s", line);
    }

    return 0;
}
