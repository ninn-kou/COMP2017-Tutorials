// 2026 S1C COMP2017 Week 7 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Trees: main.c, tree.c, tree.h


#include "tree.h"

#include <stdio.h>
#include <stdlib.h>

tree_node *create_node(int value) {
    tree_node *node = malloc(sizeof *node);
    if (node == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    node->value = value;
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;
    return node;
}

void add_child(tree_node *parent, tree_node *child) {
    if (parent == NULL || child == NULL) {
        return;
    }

    if (parent->child_count == parent->child_capacity) {
        size_t new_capacity =
            (parent->child_capacity == 0) ? 2 : parent->child_capacity * 2;

        tree_node **new_children =
            realloc(parent->children, new_capacity * sizeof *new_children);

        if (new_children == NULL) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }

        parent->children = new_children;
        parent->child_capacity = new_capacity;
    }

    parent->children[parent->child_count] = child;
    parent->child_count++;
}

void dfs(tree_node *root) {
    if (root == NULL) {
        return;
    }

    printf("%c ", root->value);

    for (size_t i = 0; i < root->child_count; i++) {
        dfs(root->children[i]);
    }
}

tree_node *find_node(tree_node *root, int value) {
    if (root == NULL) {
        return NULL;
    }

    if (root->value == value) {
        return root;
    }

    for (size_t i = 0; i < root->child_count; i++) {
        tree_node *found = find_node(root->children[i], value);
        if (found != NULL) {
            return found;
        }
    }

    return NULL;
}

void destroy_tree(tree_node *root) {
    if (root == NULL) {
        return;
    }

    for (size_t i = 0; i < root->child_count; i++) {
        destroy_tree(root->children[i]);
    }

    free(root->children);
    free(root);
}
