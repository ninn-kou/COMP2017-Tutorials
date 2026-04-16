// 2026 S1C COMP2017 Week 7 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Basic File System Creation: main.c, fc.c, fs.h


#ifndef FS_H
#define FS_H

#include <stddef.h>

typedef struct fs_node {
    char *name;
    int is_directory;
    struct fs_node *parent;
    struct fs_node **children;
    size_t child_count;
    size_t child_capacity;
} fs_node;

fs_node *fs_create_node(const char *name, int is_directory, fs_node *parent);
void fs_destroy(fs_node *node);

fs_node *fs_find_child(fs_node *dir, const char *name);
int fs_add_child(fs_node *parent, fs_node *child);
int fs_remove_child(fs_node *parent, const char *name);

void fs_ls(const fs_node *dir);
void fs_tree_print(const fs_node *node, int depth);
int fs_find_in_subtree(const fs_node *node, const char *target);
void fs_print_prompt(const fs_node *cwd);

#endif
