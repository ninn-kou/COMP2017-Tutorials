// 2026 S1C COMP2017 Week 4 Tutorial B Demonstration
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Demonstration about struct initialization.


#include <stdio.h>

struct Point {
    int x;
    int y;
};

int main(void) {

    struct Point P = {0};
    // struct Point P = {1};

    printf("%d %d\n", P.x, P.y);
    return 0;
}
