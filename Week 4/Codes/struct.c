// 2026 S1C COMP2017 Week 4 Tutorial B Demonstration
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// A sample struct.


#include <stdio.h>

struct coordinate {
    int x;
    int y;
};

int main(void) {
    struct coordinate point = {
        .x = 2,
        .y = 4
    };

    printf("point = (%d, %d)\n", point.x, point.y);

    struct coordinate* point_ptr = &point;

    point_ptr->x = 10;
    point_ptr->y = 20;

    printf("point = (%d, %d)\n", point.x, point.y);

    return 0;
}
