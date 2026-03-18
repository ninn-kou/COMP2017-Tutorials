// 2026 S1C COMP2017 Week 4 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Volume of A Sphere.


#include <stdio.h>

int main(void) {
    double radius;
    const float PI = 3.1415f;
    double volume;

    printf("Specify the radius of the sphere: ");

    if (scanf("%lf", &radius) != 1) {
        return 1;
    }

    volume = (4.0 / 3.0) * PI * radius * radius * radius;

    printf("Volume is: %lf\n", volume);

    return 0;
}
