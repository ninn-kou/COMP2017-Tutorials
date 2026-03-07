// 2026 S1C COMP2017 Week 2 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Swap two integers by *pointers*.


#include <stdio.h>

void swap(int *x, int *y){
    int tmp = *x;
    *x = *y;
    *y = tmp;
}

int main(void){
    int x = 8;
    int y = 9;

    printf("%d %d\n", x, y);
    swap(&x, &y);
    printf("%d %d\n", x, y);

    return 0;
}
