// 2026 S1C COMP2017 Week 9 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// An demonstration about pipe() to send message from child to parent.


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
        // Child process: write to the pipe.

        close(pipefd[0]);  // child does not read

        const char *message = "hello from child\n";
        write(pipefd[1], message, 17);

        close(pipefd[1]);
        return 0;
    }

    // Parent process: read from the pipe.

    close(pipefd[1]);  // parent does not write

    char buffer[100];
    ssize_t nread = read(pipefd[0], buffer, sizeof(buffer) - 1);

    if (nread == -1) {
        perror("read");
        close(pipefd[0]);
        return 1;
    }

    buffer[nread] = '\0';
    printf("parent received: %s", buffer);

    close(pipefd[0]);

    wait(NULL);

    return 0;
}
