// 2026 S1C COMP2017 Week 9 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// An example of pipeline redirection.


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
        // Child: redirect stdout to the pipe.

        close(pipefd[0]); // child does not read

        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            return 1;
        }

        close(pipefd[1]); // fd 1 now refers to the pipe, so this is no longer needed

        printf("hello from child\n");
        fflush(stdout);

        return 0;
    }

    // Parent: read from the pipe.

    close(pipefd[1]); // parent does not write

    char buffer[256];
    ssize_t nread = read(pipefd[0], buffer, sizeof(buffer) - 1);

    if (nread == -1) {
        perror("read");
        close(pipefd[0]);
        waitpid(pid, NULL, 0);
        return 1;
    }

    buffer[nread] = '\0';

    printf("parent received: %s", buffer);

    close(pipefd[0]);
    waitpid(pid, NULL, 0);

    return 0;
}
