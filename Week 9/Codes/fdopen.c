// 2026 S1C COMP2017 Week 9 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// An demonstration about fdopen().


#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
    int pipefd[2];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        close(pipefd[0]);

        dprintf(pipefd[1], "hello from child\n");

        close(pipefd[1]);
        return 0;
    }

    close(pipefd[1]);

    FILE *fp = fdopen(pipefd[0], "r");

    if (fp == NULL) {
        perror("fdopen");
        close(pipefd[0]);
        waitpid(pid, NULL, 0);
        return 1;
    }

    char line[256];

    if (fgets(line, sizeof(line), fp) != NULL) {
        printf("parent read: %s", line);
    }

    fclose(fp);
    waitpid(pid, NULL, 0);

    return 0;
}
