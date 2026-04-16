// 2026 S1C COMP2017 Week 7 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Basic File System Creation: main.c, fc.c, fs.h


#include "fs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *fs_strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (copy != NULL) {
        memcpy(copy, s, len);
    }
    return copy;
}

fs_node *fs_create_node(const char *name, int is_directory, fs_node *parent) {
    fs_node *node = malloc(sizeof *node);
    if (node == NULL) {
        return NULL;
    }

    node->name = fs_strdup(name);
    if (node->name == NULL) {
        free(node);
        return NULL;
    }

    node->is_directory = is_directory;
    node->parent = parent;
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;
    return node;
}

void fs_destroy(fs_node *node) {
    if (node == NULL) {
        return;
    }

    for (size_t i = 0; i < node->child_count; i++) {
        fs_destroy(node->children[i]);
    }

    free(node->children);
    free(node->name);
    free(node);
}

fs_node *fs_find_child(fs_node *dir, const char *name) {
    if (dir == NULL || !dir->is_directory) {
        return NULL;
    }

    for (size_t i = 0; i < dir->child_count; i++) {
        if (strcmp(dir->children[i]->name, name) == 0) {
            return dir->children[i];
        }
    }

    return NULL;
}

int fs_add_child(fs_node *parent, fs_node *child) {
    if (parent == NULL || child == NULL || !parent->is_directory) {
        return 0;
    }

    if (fs_find_child(parent, child->name) != NULL) {
        return 0;
    }

    if (parent->child_count == parent->child_capacity) {
        size_t new_capacity =
            (parent->child_capacity == 0) ? 4 : parent->child_capacity * 2;

        fs_node **new_children =
            realloc(parent->children, new_capacity * sizeof *new_children);

        if (new_children == NULL) {
            return 0;
        }

        parent->children = new_children;
        parent->child_capacity = new_capacity;
    }

    parent->children[parent->child_count] = child;
    child->parent = parent;
    parent->child_count++;
    return 1;
}

int fs_remove_child(fs_node *parent, const char *name) {
    if (parent == NULL || !parent->is_directory) {
        return 0;
    }

    for (size_t i = 0; i < parent->child_count; i++) {
        if (strcmp(parent->children[i]->name, name) == 0) {
            fs_destroy(parent->children[i]);

            for (size_t j = i + 1; j < parent->child_count; j++) {
                parent->children[j - 1] = parent->children[j];
            }

            parent->child_count--;
            return 1;
        }
    }

    return 0;
}

void fs_ls(const fs_node *dir) {
    if (dir == NULL || !dir->is_directory) {
        return;
    }

    for (size_t i = 0; i < dir->child_count; i++) {
        printf("%s", dir->children[i]->name);
        if (dir->children[i]->is_directory) {
            printf("/");
        }
        printf("\n");
    }
}

static void fs_print_path(const fs_node *node) {
    if (node == NULL) {
        return;
    }

    if (node->parent == NULL) {
        printf("%s", node->name);
        return;
    }

    fs_print_path(node->parent);
    printf("/%s", node->name);
}

void fs_print_prompt(const fs_node *cwd) {
    printf(">");
    fs_print_path(cwd);
    printf(" ");
    fflush(stdout);
}

void fs_tree_print(const fs_node *node, int depth) {
    if (node == NULL) {
        return;
    }

    if (depth == 0) {
        printf("%s\n", node->name);
    } else {
        printf("|");
        for (int i = 1; i < depth; i++) {
            printf("----");
        }
        printf("%s\n", node->name);
    }

    for (size_t i = 0; i < node->child_count; i++) {
        fs_tree_print(node->children[i], depth + 1);
    }
}

static void fs_find_recursive(const fs_node *node, const char *target, int *matches) {
    if (node == NULL) {
        return;
    }

    if (strcmp(node->name, target) == 0) {
        fs_print_path(node);
        printf("\n");
        (*matches)++;
    }

    for (size_t i = 0; i < node->child_count; i++) {
        fs_find_recursive(node->children[i], target, matches);
    }
}

int fs_find_in_subtree(const fs_node *node, const char *target) {
    int matches = 0;
    fs_find_recursive(node, target, &matches);
    return matches;
}
