// 2026 S1C COMP2017 Week 3 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Password Generator


#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <length>\n", argv[0]);
        return 1;
    }

    int length = atoi(argv[1]);
    if (length <= 0) {
        fprintf(stderr, "Length must be positive.\n");
        return 1;
    }

    // srand((unsigned)time(NULL));

    srand(1);   // Set seed.

    for (int i = 0; i < length; i++) {
        int ch = 33 + rand() % 94;   // Printable ASCII 33 to 126.
        putchar(ch);
    }
    putchar('\n');

    return 0;
}
