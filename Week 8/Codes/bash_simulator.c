// 2026 S1C COMP2017 Week 8 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// A bash "simulator" calls `execvp()` to run a command.
// This is the exercise "Bash Simulator I".
// We will expand it to "Bash Simulator II" in next week.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024
#define MAX_ARGS 64

int main(void) {
    char line[BUFFER_SIZE];

    while (1) {
        printf("mini$ ");
        fflush(stdout);

        if (fgets(line, sizeof line, stdin) == NULL) {
            printf("\n");
            break;
        }

        line[strcspn(line, "\n")] = '\0';

        if (line[0] == '\0') {
            continue;
        }

        char *argv[MAX_ARGS];
        int argc = 0;

        char *token = strtok(line, " \t");
        while (token != NULL && argc < MAX_ARGS - 1) {
            argv[argc++] = token;
            token = strtok(NULL, " \t");
        }
        argv[argc] = NULL;

        if (argc == 0) {
            continue;
        }

        if (strcmp(argv[0], "exit") == 0) {
            break;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            continue;
        }

        if (pid == 0) {
            execvp(argv[0], argv);
            perror("execvp failed");
            _exit(127);
        } else {
            if (waitpid(pid, NULL, 0) == -1) {
                perror("waitpid failed");
            }
        }
    }

    return 0;
}
