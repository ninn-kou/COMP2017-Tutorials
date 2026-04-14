// 2026 S1C COMP2017 Week 6 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Sorting with qsort().


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct fish {
    char *name;
    float length;
    int tastiness;
};

#define NUM_FISHES 10

int compare_fish(const void *a, const void *b) {
    const struct fish *fa = a;
    const struct fish *fb = b;

    // First key: length
    if (fa->length < fb->length) {
        return -1;
    }
    if (fa->length > fb->length) {
        return 1;
    }

    // Tie-break 1: tastiness
    if (fa->tastiness < fb->tastiness) {
        return -1;
    }
    if (fa->tastiness > fb->tastiness) {
        return 1;
    }

    // Tie-break 2: name
    return strcmp(fa->name, fb->name);
}

int main(void) {
    struct fish fishes[NUM_FISHES] = {
        {.name = "Salmon",  .length = 70.5,  .tastiness = 10},
        {.name = "Tuna",    .length = 120.0, .tastiness = 9},
        {.name = "Sardine", .length = 15.2,  .tastiness = 4},
        {.name = "Cod",     .length = 70.5,  .tastiness = 6},
        {.name = "Carp",    .length = 50.0,  .tastiness = 2},
        {.name = "Snapper", .length = 40.0,  .tastiness = 8},
        {.name = "Bass",    .length = 40.0,  .tastiness = 7},
        {.name = "Marlin",  .length = 120.0, .tastiness = 9},
        {.name = "Guppy",   .length = 3.5,   .tastiness = 1},
        {.name = "Trout",   .length = 50.0,  .tastiness = 2}
    };

    qsort(fishes, NUM_FISHES, sizeof fishes[0], compare_fish);

    for (int i = 0; i < NUM_FISHES; i++) {
        printf("%-7s | %-5.1f | %d\n",
               fishes[i].name,
               fishes[i].length,
               fishes[i].tastiness);
    }

    return 0;
}
