// 2026 S1C COMP2017 Week 3 Tutorial A Demonstration
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// To echo user's all arguments.


#include <stdio.h>

int main(int argc, char *argv[]) {
    int i;

    for (i = 1; i < argc; i++) {
        printf("%s", argv[i]);

        if (i < argc - 1) {
            printf(" ");
        }
    }

    printf("\n");

    return 0;
}
