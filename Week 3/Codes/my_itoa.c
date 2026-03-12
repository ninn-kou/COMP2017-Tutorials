// 2026 S1C COMP2017 Week 3 Tutorial A Extension
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Convert an integer to a string.


#include <stdio.h>

void reverse(char s[]) {
    int i = 0;
    int j = 0;
    char temp;

    /* Find the end of the string */
    while (s[j] != '\0') {
        j++;
    }
    j--;   /* move to the last real character */

    /* Swap from both ends */
    while (i < j) {
        temp = s[i];
        s[i] = s[j];
        s[j] = temp;
        i++;
        j--;
    }
}

void my_itoa(int n, char s[]) {
    int i = 0;
    int negative = 0;
    long long value = n;   /* use a larger type for safety */

    if (value < 0) {
        negative = 1;
        value = -value;
    }

    /* Special case is handled naturally by do...while */
    do {
        int digit = (int)(value % 10);
        s[i++] = digit + '0';
        value /= 10;
    } while (value > 0);

    if (negative) {
        s[i++] = '-';
    }

    s[i] = '\0';

    reverse(s);
}

int main(void) {
    char s[50];

    my_itoa(0, s);
    printf("%s\n", s);

    my_itoa(1234, s);
    printf("%s\n", s);

    my_itoa(-1234, s);
    printf("%s\n", s);

    my_itoa(42, s);
    printf("%s\n", s);

    my_itoa(-7, s);
    printf("%s\n", s);

    my_itoa(1000, s);
    printf("%s\n", s);

    return 0;
}
