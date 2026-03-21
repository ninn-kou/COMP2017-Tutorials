// 2026 S1C COMP2017 Week 4 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// An shape interface simulated Java one.


#include <assert.h>
#include <math.h>
#include <stdio.h>

enum ShapeType { CIRCLE, RECTANGLE, TRIANGLE };

struct Shape {
    enum ShapeType type;
    union {
        struct {
            float radius;
        } circle;

        struct {
            float width;
            float height;
        } rectangle;

        struct {
            float base;
            float height;
        } triangle;
    } data;
};

float area(struct Shape *ptr) {
    assert(ptr != NULL);

    switch (ptr->type) {
        case CIRCLE:
            return 3.14159f * ptr->data.circle.radius * ptr->data.circle.radius;

        case RECTANGLE:
            return ptr->data.rectangle.width * ptr->data.rectangle.height;

        case TRIANGLE:
            return 0.5f * ptr->data.triangle.base * ptr->data.triangle.height;

        default:
            assert(0 && "Unknown shape type");
            return 0.0f;
    }
}

int main(void) {
    const float alpha = 0.0001f;

    // Test circle
    struct Shape circle;
    circle.type = CIRCLE;
    circle.data.circle.radius = 2.0f;

    float circle_area = area(&circle);
    assert(fabsf(circle_area - (3.14159f * 2.0f * 2.0f)) < alpha);

    // Test rectangle
    struct Shape rectangle;
    rectangle.type = RECTANGLE;
    rectangle.data.rectangle.width = 3.0f;
    rectangle.data.rectangle.height = 4.0f;

    float rectangle_area = area(&rectangle);
    assert(fabsf(rectangle_area - 12.0f) < alpha);

    // Test triangle
    struct Shape triangle;
    triangle.type = TRIANGLE;
    triangle.data.triangle.base = 10.0f;
    triangle.data.triangle.height = 6.0f;

    float triangle_area = area(&triangle);
    assert(fabsf(triangle_area - 30.0f) < alpha);

    printf("All tests passed.\n");
    return 0;
}
