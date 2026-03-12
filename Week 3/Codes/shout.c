// 2026 S1C COMP2017 Week 3 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Shout (Lower Case Letters to Upper Case Letters)


#include <stdio.h>
#include <ctype.h>

int main(void)
{
    int ch;

    while ((ch = getchar()) != EOF) {
        /*
        if (ch >= 'a' && ch <= 'z') {
            ch = ch - ('a' - 'A');
        }
        putchar(ch);
        */
        putchar(toupper((unsigned char) ch));
    }

    return 0;
}
