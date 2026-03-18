// 2026 S1C COMP2017 Week 4 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Simple "my_strstr()"" function.


#include <stdio.h>

int substring(const char* line, const char* substr) {
    int i, j;

    if (substr[0] == '\0') {
        return 0;   // empty substring matches at the start
    }

    for (i = 0; line[i] != '\0'; i++) {
        j = 0;

        while (substr[j] != '\0' &&
               line[i + j] != '\0' &&
               line[i + j] == substr[j]) {
            j++;
        }

        if (substr[j] == '\0') {
            return i;   // found a complete match
        }
    }

    return -1;          // substring not found
}

int main(void) {
    printf("%d\n", substring("racecar", "car"));   // 4
    printf("%d\n", substring("telephone", "one")); // 6
    printf("%d\n", substring("monkey", "cat"));    // -1

    // extra tests
    printf("%d\n", substring("banana", "ana"));    // 1
    printf("%d\n", substring("aaaaa", "aa"));      // 0
    printf("%d\n", substring("abc", ""));          // 0
    printf("%d\n", substring("", "a"));            // -1

    return 0;
}
