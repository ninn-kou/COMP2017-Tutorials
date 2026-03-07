// 2026 S1C COMP2017 Week 2 Tutorial B Demonstration
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Copy from Ed Lesson Week 2

// Explain original outputs first, then "Demo 1", "Demo 2" and "Demo 3".


#include <stdio.h>

int unarray(char arr[]) {
    // how does sizeof behave???
    printf("%zu\n", sizeof(arr));
}

int main(){
    const char * ptr = "hello";
    const char array[] = "hello";
    const char array2[] = { 'h', 'e', 'l', 'l', 'o' };
    const char array3[] = { 'h', 'e', 'l', 'l', 'o', '\0' };

    const char array4[5] = { 'h', 'e', 'l', 'l', 'o' };
    const char array5[6] = { 'h', 'e', 'l', 'l', 'o', 0 };
    const char array6[20] = { 'h', 'e', 'l', 'l', 'o' };
    const char array7[20] = { 0 };
    const char array8[20] = "hello";

    const char array9[10][10] = { 0 };

    printf("%zu %zu\n", sizeof(ptr),     sizeof(array));
    printf("%zu %zu\n", sizeof(array2),  sizeof(array3));
    printf("%zu %zu\n", sizeof(*ptr),    sizeof(&array));
    printf("%zu %zu\n", sizeof(&array2), sizeof(&array3));

    /* Demo 2
    printf("--------\n");
    printf("%zu %zu %zu %zu %zu\n", sizeof(array4), sizeof(array5), sizeof(array6), sizeof(array7), sizeof(array8));
    printf("%zu %zu\n", sizeof(array9),  sizeof(&array9));
    /*

    /* Demo 1
    printf("--------\n");
    printf("%zu %zu\n", sizeof(char),   sizeof(char*));
    printf("%zu %zu\n", sizeof(int),    sizeof(int*));
    printf("%zu %zu\n", sizeof(double), sizeof(double*));
    */

    /* Demo 3
    printf("%zu\n", sizeof(unarray));
    printf("%zu\n", sizeof(void*));
    */

    /* Demo 4
    printf("--------\n");
    unarray(array);
    unarray(array2);
    unarray(array3);
    unarray(array4);
    unarray(array5);
    unarray(array6);
    unarray(array7);
    unarray(array8);
    */
}