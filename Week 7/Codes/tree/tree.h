// 2026 S1C COMP2017 Week 7 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Trees: main.c, tree.c, tree.h


#ifndef TREE_H
#define TREE_H

#include <stddef.h>

typedef struct tree_node {
    int value;
    struct tree_node **children;
    size_t child_count;
    size_t child_capacity;
} tree_node;

tree_node *create_node(int value);
void add_child(tree_node *parent, tree_node *child);
void dfs(tree_node *root);
tree_node *find_node(tree_node *root, int value);
void destroy_tree(tree_node *root);

#endif
