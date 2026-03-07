// 2026 S1C COMP2017 Week 2 Tutorial A & B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Demonstration for `getchar()`, `scanf()` and `fgets()`.


#include <stdio.h>
#include <string.h> // Only required by Solution 3

//------------------------------------------------------------------------------
// Solution 1 by using getchar()

int main(void) {
    int c;  // int is required so we can represent all chars + EOF
    while ((c = getchar()) != EOF) {
        if (putchar(c) == EOF) {   // write to stdout
            return 1;
        }
    }
    return 0;
}


// -----------------------------------------------------------------------------
// Solution 2 by using scanf()

// int main(void) {
//     char line[100];

//     // Reads up to 99 chars that are not '\n'
//     if (scanf("%99[^\n]", line) == 1) {
//         printf("%s\n", line);
//     }
//     return 0;
// }


// -----------------------------------------------------------------------------
// Solution 3 by using fgets()

// int main(void) {
//     char buf[1024];

//     while (fgets(buf, sizeof buf, stdin)) {
//         buf[strcspn(buf, "\n")] = '\0'; // remove trailing newline if any
//         printf("%s\n", buf);
//     }
// }

