// 2026 S1C COMP2017 Week 5 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Given a singly linked list, retrieve the k-th last element.


#include <stdio.h>
#include <stdlib.h>

struct node {
    int value;
    struct node *next;
};

struct node* list_kth_last(struct node *head, int k) {
    if (head == NULL || k <= 0) {
        return NULL;
    }

    struct node *slow = head;
    struct node *fast = head;

    for (int i = 0; i < k; i++) {
        if (fast == NULL) {
            return NULL;
        }
        fast = fast->next;
    }

    while (fast != NULL) {
        slow = slow->next;
        fast = fast->next;
    }

    return slow;
}

int main(void) {
    struct node n4 = {4, NULL};
    struct node n3 = {3, &n4};
    struct node n2 = {2, &n3};
    struct node n1 = {1, &n2};

    struct node *result = list_kth_last(&n1, 2);

    if (result != NULL) {
        printf("%d\n", result->value);   // prints 3
    } else {
        printf("Invalid k\n");
    }

    return 0;
}
