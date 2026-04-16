// 2026 S1C COMP2017 Week 7 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Basic File System Creation: main.c, fc.c, fs.h


#include <stdio.h>
#include <string.h>

#include "fs.h"

int main(void) {
    fs_node *root = fs_create_node("~", 1, NULL);
    if (root == NULL) {
        fprintf(stderr, "Could not create root\n");
        return 1;
    }

    fs_node *cwd = root;
    char line[256];

    while (1) {
        fs_print_prompt(cwd);

        if (fgets(line, sizeof line, stdin) == NULL) {
            break;
        }

        line[strcspn(line, "\n")] = '\0';

        if (line[0] == '\0') {
            continue;
        }

        char *cmd = strtok(line, " ");
        if (cmd == NULL) {
            continue;
        }

        if (strcmp(cmd, "touch") == 0) {
            char *name = strtok(NULL, " ");
            if (name == NULL) {
                printf("Usage: touch <fname>\n");
                continue;
            }

            fs_node *file = fs_create_node(name, 0, cwd);
            if (file == NULL || !fs_add_child(cwd, file)) {
                printf("Could not create file\n");
                fs_destroy(file);
            }
        } else if (strcmp(cmd, "mkdir") == 0) {
            char *name = strtok(NULL, " ");
            if (name == NULL) {
                printf("Usage: mkdir <dname>\n");
                continue;
            }

            fs_node *dir = fs_create_node(name, 1, cwd);
            if (dir == NULL || !fs_add_child(cwd, dir)) {
                printf("Could not create directory\n");
                fs_destroy(dir);
            }
        } else if (strcmp(cmd, "ls") == 0) {
            fs_ls(cwd);
        } else if (strcmp(cmd, "cd") == 0) {
            char *arg = strtok(NULL, " ");
            if (arg == NULL) {
                printf("Usage: cd <name|..>\n");
                continue;
            }

            if (strcmp(arg, "..") == 0) {
                if (cwd->parent != NULL) {
                    cwd = cwd->parent;
                } else {
                    printf("Already at root\n");
                }
            } else {
                fs_node *next = fs_find_child(cwd, arg);
                if (next == NULL || !next->is_directory) {
                    printf("No such directory\n");
                } else {
                    cwd = next;
                }
            }
        } else if (strcmp(cmd, "rm") == 0) {
            char *name = strtok(NULL, " ");
            if (name == NULL) {
                printf("Usage: rm <name>\n");
                continue;
            }

            if (!fs_remove_child(cwd, name)) {
                printf("No such file or directory\n");
            }
        } else if (strcmp(cmd, "find") == 0) {
            char *name = strtok(NULL, " ");
            if (name == NULL) {
                printf("Usage: find <name>\n");
                continue;
            }

            if (fs_find_in_subtree(cwd, name) == 0) {
                printf("Not found\n");
            }
        } else if (strcmp(cmd, "tree") == 0) {
            fs_tree_print(cwd, 0);
        } else if (strcmp(cmd, "exit") == 0) {
            break;
        } else {
            printf("Unknown command\n");
        }
    }

    fs_destroy(root);
    return 0;
}
