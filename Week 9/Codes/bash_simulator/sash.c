// 2026 S1C COMP2017 Week 9 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Bash Simulator: sash.c, commands.c, commands.h


#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "commands.h"

#define MAX_INPUT 256

/*
 * These globals are used so spawn_child() can close all pipe file descriptors
 * inherited by the child.
 *
 * This keeps the scaffold's original spawn_child signature:
 *
 *     spawn_child(command, in_num, out_num)
 */
static int (*active_pipes)[2] = NULL;
static int active_pipe_count = 0;

/*
 * Calculates the length of a NULL-terminated array.
 */
int array_length(void **array) {
    if (array == NULL) {
        return 0;
    }

    int i = 0;

    while (array[i] != NULL) {
        i++;
    }

    return i;
}

static void close_pipe_array(int pipes[][2], int num_pipes) {
    if (pipes == NULL) {
        return;
    }

    for (int i = 0; i < num_pipes; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
}

/*
 * Spawns a child process to execute the given command.
 *
 * Returns the child's pid in the parent.
 *
 * in_num and out_num are the file descriptors to map to
 * STDIN_FILENO and STDOUT_FILENO inside the child.
 */
pid_t spawn_child(char **command, int in_num, int out_num) {
    if (command == NULL || command[0] == NULL) {
        errno = EINVAL;
        return -1;
    }

    pid_t pid = fork();

    if (pid == -1) {
        return -1;
    }

    if (pid == 0) {
        /*
         * Child process.
         *
         * If in_num is not normal stdin, make stdin read from in_num.
         */
        if (in_num != STDIN_FILENO) {
            if (dup2(in_num, STDIN_FILENO) == -1) {
                perror("dup2 stdin");
                _exit(126);
            }
        }

        /*
         * If out_num is not normal stdout, make stdout write to out_num.
         */
        if (out_num != STDOUT_FILENO) {
            if (dup2(out_num, STDOUT_FILENO) == -1) {
                perror("dup2 stdout");
                _exit(126);
            }
        }

        /*
         * After dup2(), the child no longer needs the original pipe
         * file descriptors.
         *
         * This is important. If a process accidentally keeps a pipe end open,
         * another command may wait forever for EOF.
         */
        close_pipe_array(active_pipes, active_pipe_count);

        /*
         * Replace this child process with the requested program.
         *
         * execvp() searches PATH, so "ls" works instead of requiring "/bin/ls".
         */
        execvp(command[0], command);

        /*
         * execvp() only returns if it failed.
         */
        perror(command[0]);
        _exit(127);
    }

    /*
     * Parent process.
     */
    return pid;
}

static int is_blank_line(const char *line) {
    if (line == NULL) {
        return 1;
    }

    for (int i = 0; line[i] != '\0'; i++) {
        if (!isspace((unsigned char) line[i])) {
            return 0;
        }
    }

    return 1;
}

static void remove_trailing_newline(char *line) {
    if (line == NULL) {
        return;
    }

    size_t len = strlen(line);

    if (len > 0 && line[len - 1] == '\n') {
        line[len - 1] = '\0';
    }
}

static int is_exit_command(char ***commands_array) {
    return commands_array != NULL &&
           commands_array[0] != NULL &&
           commands_array[0][0] != NULL &&
           strcmp(commands_array[0][0], "exit") == 0 &&
           commands_array[0][1] == NULL &&
           commands_array[1] == NULL;
}

static int wait_for_child(pid_t child) {
    int status;

    while (waitpid(child, &status, 0) == -1) {
        if (errno != EINTR) {
            perror("waitpid");
            return 1;
        }
    }

    return 0;
}

static int run_pipeline(char ***commands_array, int num_commands) {
    if (commands_array == NULL || num_commands <= 0) {
        return 0;
    }

    for (int i = 0; i < num_commands; i++) {
        if (commands_array[i] == NULL || commands_array[i][0] == NULL) {
            fprintf(stderr, "SASH: empty command\n");
            return 1;
        }
    }

    pid_t *children = calloc((size_t) num_commands, sizeof(pid_t));

    if (children == NULL) {
        perror("calloc children");
        return 1;
    }

    int num_pipes = num_commands - 1;
    int (*pipes)[2] = NULL;

    if (num_pipes > 0) {
        pipes = malloc(sizeof(int[2]) * (size_t) num_pipes);

        if (pipes == NULL) {
            perror("malloc pipes");
            free(children);
            return 1;
        }

        for (int i = 0; i < num_pipes; i++) {
            if (pipe(pipes[i]) == -1) {
                perror("pipe");

                close_pipe_array(pipes, i);

                free(pipes);
                free(children);

                return 1;
            }
        }
    }

    /*
     * Make the pipe array visible to spawn_child().
     * Each child will inherit these values after fork().
     */
    active_pipes = pipes;
    active_pipe_count = num_pipes;

    int children_spawned = 0;

    for (int i = 0; i < num_commands; i++) {
        int in_num = STDIN_FILENO;
        int out_num = STDOUT_FILENO;

        /*
         * Not the first command:
         * read from the previous pipe.
         */
        if (i > 0) {
            in_num = pipes[i - 1][0];
        }

        /*
         * Not the last command:
         * write to the next pipe.
         */
        if (i < num_commands - 1) {
            out_num = pipes[i][1];
        }

        children[i] = spawn_child(commands_array[i], in_num, out_num);

        if (children[i] == -1) {
            perror("spawn_child");

            close_pipe_array(pipes, num_pipes);

            active_pipes = NULL;
            active_pipe_count = 0;

            for (int j = 0; j < children_spawned; j++) {
                wait_for_child(children[j]);
            }

            free(pipes);
            free(children);

            return 1;
        }

        children_spawned++;
    }

    /*
     * The parent does not take part in the pipeline.
     *
     * After spawning all children, it should close every pipe fd.
     * Otherwise, commands like "ls | wc -l" may hang because wc never sees EOF.
     */
    close_pipe_array(pipes, num_pipes);

    active_pipes = NULL;
    active_pipe_count = 0;

    /*
     * Wait after spawning all children, not after each individual child.
     * Pipeline commands should run concurrently.
     */
    for (int i = 0; i < children_spawned; i++) {
        wait_for_child(children[i]);
    }

    free(pipes);
    free(children);

    return 0;
}

int main(void) {
    size_t max_input_size = MAX_INPUT;
    char *input_buffer = malloc(MAX_INPUT);

    if (input_buffer == NULL) {
        perror("malloc input_buffer");
        return 1;
    }

    memset(input_buffer, 0, MAX_INPUT);

    while (1) {
        printf("SASH> ");
        fflush(stdout);

        ssize_t read_bytes = getline(&input_buffer, &max_input_size, stdin);

        if (read_bytes == -1) {
            break;
        }

        /*
         * Remove the newline before parsing.
         *
         * The scaffold removed it after deconstruct_command(), but that is too late:
         * the final argument may already contain the newline.
         */
        remove_trailing_newline(input_buffer);

        if (is_blank_line(input_buffer)) {
            continue;
        }

        /*
         * deconstruct_command() edits input_buffer in place.
         */
        char ***commands_array = deconstruct_command(input_buffer);

        if (commands_array == NULL) {
            continue;
        }

        int num_commands = array_length((void **) commands_array);

        /*
         * Small shell builtin so the program can exit cleanly.
         */
        if (is_exit_command(commands_array)) {
            free_decon_command(commands_array);
            break;
        }

        run_pipeline(commands_array, num_commands);

        /*
         * This frees the arrays created by deconstruct_command().
         * It does not free input_buffer, because the command words point into it.
         */
        free_decon_command(commands_array);
    }

    free(input_buffer);

    return 0;
}
