// 2026 S1C COMP2017 Week 4 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Simple "my_strncmp()"" function.


#include <stdio.h>

int string_n_compare(const char* w1, const char* w2, unsigned int len) {
    unsigned int i = 0;

    while (i < len && w1[i] == w2[i]) {
        if (w1[i] == '\0') {
            return 0;   // both strings ended here
        }
        i++;
    }

    if (i == len) {
        return 0;       // first len characters are equal
    }

    return (unsigned char)w1[i] - (unsigned char)w2[i];
}

int main(void) {
    printf("%d\n", string_n_compare("Hello", "Hello", 5));    // 0
    printf("%d\n", string_n_compare("Hello", "Byebye", 5));   // 6 = 'H' - 'B' = 72 - 66
    printf("%d\n", string_n_compare("Hello", "byebye", 5));   // -26 = 'H' - 'b' = 72 - 98
    printf("%d\n", string_n_compare("Hello", "Helium", 3));   // 0
    printf("%d\n", string_n_compare("cat", "catalog", 3));    // 0
    printf("%d\n", string_n_compare("cat", "catalog", 4));    // -97 = '\0' - 'a' = 0 - 97
    printf("%d\n", string_n_compare("abc", "abd", 0));        // 0
    printf("%d\n", string_n_compare("", "", 10));             // 0

    return 0;
}
