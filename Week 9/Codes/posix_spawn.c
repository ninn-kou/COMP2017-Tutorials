// 2026 S1C COMP2017 Week 9 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// A demo for the function `posix_spawn()` with `ls -l`.


#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

int main(void) {
    pid_t pid;
    int status;
    posix_spawn_file_actions_t actions;

    char *argv[] = {
        "ls",
        "-l",
        NULL
    };

    int err = posix_spawn_file_actions_init(&actions);

    if (err != 0) {
        errno = err;
        perror("posix_spawn_file_actions_init");
        return EXIT_FAILURE;
    }

    err = posix_spawn_file_actions_addopen(
        &actions,
        STDOUT_FILENO,
        "output.txt",
        O_WRONLY | O_CREAT | O_TRUNC,
        0644
    );

    if (err != 0) {
        errno = err;
        perror("posix_spawn_file_actions_addopen");
        posix_spawn_file_actions_destroy(&actions);
        return EXIT_FAILURE;
    }

    err = posix_spawnp(&pid, "ls", &actions, NULL, argv, environ);

    posix_spawn_file_actions_destroy(&actions);

    if (err != 0) {
        errno = err;
        perror("posix_spawnp");
        return EXIT_FAILURE;
    }

    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
