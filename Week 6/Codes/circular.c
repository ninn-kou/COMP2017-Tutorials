// 2026 S1C COMP2017 Week 6 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Circular Linked List - Expand from Week 5 Tutorial B `list.c`.


#include <stdio.h>
#include <stdlib.h>

typedef struct node node;

struct node {
    int data;
    node *prev;
    node *next;
};

node* list_init(int value);
void list_add(node** head, int value);
void list_delete(node** head, node* n);
node* list_next(const node* n);
void list_free(node** head);
void list_print(const node *head);


int main(void) {
    node *head = list_init(10);
    list_add(&head, 20);
    list_add(&head, 30);
    list_add(&head, 40);

    list_print(head);   // 10 -> 20 -> 30 -> 40 -> (back to head)

    node *second = list_next(head);
    list_delete(&head, second);

    list_print(head);   // 10 -> 30 -> 40 -> (back to head)

    list_delete(&head, head);
    list_print(head);   // 30 -> 40 -> (back to head)

    list_free(&head);
    list_print(head);   // NULL

    return 0;
}

////////////////////////////////////////////////////////////////////////////////

node* list_init(int value) {
    node *head = malloc(sizeof *head);
    if (head == NULL) {
        return NULL;
    }

    head->data = value;
    head->next = head;
    head->prev = head;
    return head;
}

void list_add(node** head, int value) {
    if (head == NULL) {
        return;
    }

    if (*head == NULL) {
        *head = list_init(value);
        return;
    }

    node *new_node = malloc(sizeof *new_node);
    if (new_node == NULL) {
        return;
    }

    node *tail = (*head)->prev;

    new_node->data = value;
    new_node->next = *head;
    new_node->prev = tail;

    tail->next = new_node;
    (*head)->prev = new_node;
}

void list_delete(node** head, node* n) {
    if (head == NULL || *head == NULL || n == NULL) {
        return;
    }

    // Case 1: only one node in the list
    if (n->next == n && n->prev == n) {
        free(n);
        *head = NULL;
        return;
    }

    // Reconnect neighbours
    n->prev->next = n->next;
    n->next->prev = n->prev;

    // If deleting the head, move head forward
    if (*head == n) {
        *head = n->next;
    }

    free(n);
}

node* list_next(const node* n) {
    if (n == NULL) {
        return NULL;
    }

    return n->next;
}

void list_free(node** head) {
    if (head == NULL || *head == NULL) {
        return;
    }

    node *start = *head;
    node *curr = start->next;

    while (curr != start) {
        node *next = curr->next;
        free(curr);
        curr = next;
    }

    free(start);
    *head = NULL;
}

void list_print(const node *head) {
    if (head == NULL) {
        printf("NULL\n");
        return;
    }

    const node *curr = head;
    do {
        printf("%d -> ", curr->data);
        curr = curr->next;
    } while (curr != head);

    printf("(back to head)\n");
}
