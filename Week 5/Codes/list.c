// 2026 S1C COMP2017 Week 5 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Singly Linked List.


#include <stdio.h>
#include <stdlib.h>

struct node {
    int value;
    struct node *next;
};

struct node* list_init(int value) {
    struct node *head = malloc(sizeof(struct node));
    if (head == NULL) {
        return NULL;
    }

    head->value = value;
    head->next = NULL;
    return head;
}

void list_add(struct node** head, int value) {
    if (head == NULL) {
        return;
    }

    struct node *new_node = malloc(sizeof(struct node));
    if (new_node == NULL) {
        return;
    }

    new_node->value = value;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
        return;
    }

    struct node *curr = *head;
    while (curr->next != NULL) {
        curr = curr->next;
    }

    curr->next = new_node;
}

void list_delete(struct node** head, struct node* n) {
    if (head == NULL || *head == NULL || n == NULL) {
        return;
    }

    if (*head == n) {
        *head = n->next;
        free(n);
        return;
    }

    struct node *curr = *head;
    while (curr->next != NULL && curr->next != n) {
        curr = curr->next;
    }

    if (curr->next == n) {
        curr->next = n->next;
        free(n);
    }
}

struct node* list_next(const struct node* n) {
    if (n == NULL) {
        return NULL;
    }

    return n->next;
}

void list_free(struct node** head) {
    if (head == NULL) {
        return;
    }

    struct node *curr = *head;
    while (curr != NULL) {
        struct node *next = curr->next;
        free(curr);
        curr = next;
    }

    *head = NULL;
}
