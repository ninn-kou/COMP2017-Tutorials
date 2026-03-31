// 2026 S1C COMP2017 Week 6 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Generic Lists Data Structure e.g. [12, "hi", 9.6].


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TYPE_INT,
    TYPE_DOUBLE,
    TYPE_STRING
} ValueType;

typedef struct Node Node;

struct Node {
    ValueType type;
    union {
        int as_int;
        double as_double;
        char *as_string;
    } data;
    Node *next;
};

static char *string_duplicate(const char *s);
static Node *node_create_int(int value);
static Node *node_create_double(double value);
static Node *node_create_string(const char *value);
static void list_append_node(Node **head, Node *new_node);
void list_add_int(Node **head, int value);
void list_add_double(Node **head, double value);
void list_add_string(Node **head, const char *value);
void list_print(const Node *head);
void list_free(Node **head);


int main(void) {
    Node *list = NULL;

    list_add_int(&list, 12);
    list_add_string(&list, "hi");
    list_add_double(&list, 9.6);

    list_print(list);   // [12, "hi", 9.6]

    list_free(&list);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

static char *string_duplicate(const char *s) {
    if (s == NULL) {
        return NULL;
    }

    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, s, len);
    return copy;
}

static Node *node_create_int(int value) {
    Node *n = malloc(sizeof *n);
    if (n == NULL) {
        return NULL;
    }

    n->type = TYPE_INT;
    n->data.as_int = value;
    n->next = NULL;
    return n;
}

static Node *node_create_double(double value) {
    Node *n = malloc(sizeof *n);
    if (n == NULL) {
        return NULL;
    }

    n->type = TYPE_DOUBLE;
    n->data.as_double = value;
    n->next = NULL;
    return n;
}

static Node *node_create_string(const char *value) {
    Node *n = malloc(sizeof *n);
    if (n == NULL) {
        return NULL;
    }

    char *copy = string_duplicate(value);
    if (copy == NULL) {
        free(n);
        return NULL;
    }

    n->type = TYPE_STRING;
    n->data.as_string = copy;
    n->next = NULL;
    return n;
}

static void list_append_node(Node **head, Node *new_node) {
    if (head == NULL || new_node == NULL) {
        return;
    }

    if (*head == NULL) {
        *head = new_node;
        return;
    }

    Node *curr = *head;
    while (curr->next != NULL) {
        curr = curr->next;
    }

    curr->next = new_node;
}

void list_add_int(Node **head, int value) {
    list_append_node(head, node_create_int(value));
}

void list_add_double(Node **head, double value) {
    list_append_node(head, node_create_double(value));
}

void list_add_string(Node **head, const char *value) {
    list_append_node(head, node_create_string(value));
}

void list_print(const Node *head) {
    printf("[");

    const Node *curr = head;
    while (curr != NULL) {
        switch (curr->type) {
            case TYPE_INT:
                printf("%d", curr->data.as_int);
                break;

            case TYPE_DOUBLE:
                printf("%g", curr->data.as_double);
                break;

            case TYPE_STRING:
                printf("\"%s\"", curr->data.as_string);
                break;
        }

        if (curr->next != NULL) {
            printf(", ");
        }

        curr = curr->next;
    }

    printf("]\n");
}

void list_free(Node **head) {
    if (head == NULL) {
        return;
    }

    Node *curr = *head;
    while (curr != NULL) {
        Node *next = curr->next;

        if (curr->type == TYPE_STRING) {
            free(curr->data.as_string);
        }

        free(curr);
        curr = next;
    }

    *head = NULL;
}
