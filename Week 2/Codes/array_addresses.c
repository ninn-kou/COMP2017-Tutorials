// 2026 S1C COMP2017 Week 2 Tutorial A Extension Codes
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Arrays, pointers and their addresses.


#include <stdio.h>
#define ARRAY_LENGTH 5

int main(void) {
    int array[ARRAY_LENGTH] = {42, 2017, 2026, 1, 0};
    int *p = array;   // same as &array[0]

    printf("Array elements:\n");
    for (int i = 0; i < ARRAY_LENGTH; i++) {
        printf("array[%d] = %d, stored at address %p\n",
               i, array[i], (void *)&array[i]);
    }

    printf("\nPointer p:\n");
    printf("p = %p, so p points to array[0]\n", (void *)p);
    printf("*p = %d, the value stored at the address in p\n", *p);
    printf("&p = %p, the address of the pointer variable itself\n", (void *)&p);

    printf("\nArray addresses:\n");
    printf("array     = %p, which becomes &array[0] in this expression\n",
           (void *)array);
    printf("&array[0] = %p, the address of the first element\n",
           (void *)&array[0]);
    printf("&array    = %p, the address of the whole array\n",
           (void *)&array);

    return 0;
}
