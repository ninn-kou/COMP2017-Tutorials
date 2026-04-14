// 2026 S1C COMP2017 Week 7 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// A `filter_ships()` function that takes in the linked list and modifies the
// list in place by removing elements that are false for the given predicate
// function.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ship {
    char *name;
    int fuel;
    float distance;
    struct ship *next;
};

// That means `ship_predicate` is the type "pointer to a function taking
// `const struct ship *` and returning `int`".
typedef int (*ship_predicate)(const struct ship *);

int has_enough_fuel(const struct ship *s) {
    return s->fuel >= 50;
}

int has_enough_distance(const struct ship *s) {
    return s->distance >= 100.0f;
}

struct ship *filter_ships(struct ship *head, ship_predicate predicate) {
    // Remove bad nodes from the front of the list
    while (head != NULL && !predicate(head)) {
        head = head->next;
    }

    // If every node was removed
    if (head == NULL) {
        return NULL;
    }

    // Remove bad nodes after the head
    struct ship *curr = head;
    while (curr->next != NULL) {
        if (!predicate(curr->next)) {
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }

    return head;
}

void print_ships(struct ship *head) {
    while (head) {
        printf("%s %d %f\n", head->name, head->fuel, head->distance);
        head = head->next;
    }
    printf("\n");
}

int main(void) {
    struct ship s1 = {"Falcon", 50, 12.0, NULL};
    struct ship s2 = {"Voyager", 10, 900.0, &s1};
    struct ship s3 = {"Enterprise", 85, 120.5, &s2};
    struct ship s4 = {"Apollo", 15, 4.2, &s3};
    struct ship *fleet = &s4;

    // filter out ships that have less than 50 fuel
    fleet = filter_ships(fleet, has_enough_fuel);
    print_ships(fleet);

    // filter out ships that are less than 100 distance
    fleet = filter_ships(fleet, has_enough_distance);
    print_ships(fleet);

    return 0;
}
