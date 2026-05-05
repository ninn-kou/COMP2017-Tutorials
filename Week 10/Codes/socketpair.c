// 2026 S1C COMP2017 Week 10 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// A demonstration about `socketpair()` usage.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>

int main(void) {
    int sv[2];
    char buf[100];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
        perror("socketpair");
        return 1;
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Child process
        close(sv[0]);   // child uses sv[1]

        ssize_t n = read(sv[1], buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            printf("Child received: %s\n", buf);
        }

        const char *reply = "Hello from child";
        write(sv[1], reply, strlen(reply));

        close(sv[1]);
        return 0;
    } else {
        // Parent process
        close(sv[1]);   // parent uses sv[0]

        const char *msg = "Hello from parent";
        write(sv[0], msg, strlen(msg));

        ssize_t n = read(sv[0], buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            printf("Parent received: %s\n", buf);
        }

        close(sv[0]);
        wait(NULL);
    }

    return 0;
}
