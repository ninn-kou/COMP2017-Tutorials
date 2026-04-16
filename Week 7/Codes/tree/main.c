// 2026 S1C COMP2017 Week 7 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Trees: main.c, tree.c, tree.h


#include <stdio.h>

#include "tree.h"

int main(void) {
    tree_node *a = create_node('A');
    tree_node *b = create_node('B');
    tree_node *c = create_node('C');
    tree_node *d = create_node('D');
    tree_node *e = create_node('E');
    tree_node *f = create_node('F');

    add_child(a, b);
    add_child(a, c);
    add_child(a, d);
    add_child(b, e);
    add_child(c, f);

    dfs(a);
    printf("\n");

    tree_node *found = find_node(a, 'F');
    if (found != NULL) {
        printf("Found: %c\n", found->value);
    }

    destroy_tree(a);
    return 0;
}
