// 2026 S1C COMP2017 Week 9 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// One parent process and one child process to communicate using pipes.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#define BUF_SIZE 256

static int write_all(int fd, const char *buf, size_t len) {
    size_t written = 0;

    while (written < len) {
        ssize_t n = write(fd, buf + written, len - written);

        if (n == -1) {
            return -1;
        }

        if (n == 0) {
            return -1;
        }

        written += (size_t)n;
    }

    return 0;
}

static int write_string(int fd, const char *s) {
    return write_all(fd, s, strlen(s) + 1);
}

int main(void) {
    int parent_to_child[2];
    int child_to_parent[2];

    if (pipe(parent_to_child) == -1) {
        perror("pipe parent_to_child");
        return 1;
    }

    if (pipe(child_to_parent) == -1) {
        perror("pipe child_to_parent");
        close(parent_to_child[0]);
        close(parent_to_child[1]);
        return 1;
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");

        close(parent_to_child[0]);
        close(parent_to_child[1]);
        close(child_to_parent[0]);
        close(child_to_parent[1]);

        return 1;
    }

    if (pid == 0) {
        /*
         * Child process:
         * - reads the parent's question
         * - prints the time message
         * - sends a response back to the parent
         */

        close(parent_to_child[1]);   // child does not write to parent_to_child
        close(child_to_parent[0]);   // child does not read from child_to_parent

        char question[BUF_SIZE];

        ssize_t nread = read(parent_to_child[0], question, sizeof(question) - 1);

        if (nread == -1) {
            perror("child read");
            close(parent_to_child[0]);
            close(child_to_parent[1]);
            return 1;
        }

        if (nread == 0) {
            fprintf(stderr, "child: no question received\n");
            close(parent_to_child[0]);
            close(child_to_parent[1]);
            return 1;
        }

        question[nread] = '\0';

        close(parent_to_child[0]);

        time_t t;

        if (time(&t) == (time_t)-1) {
            perror("time");
            close(child_to_parent[1]);
            return 1;
        }

        struct tm *tm_info = localtime(&t);

        if (tm_info == NULL) {
            perror("localtime");
            close(child_to_parent[1]);
            return 1;
        }

        char time_buf[BUF_SIZE];

        if (strftime(time_buf, sizeof(time_buf), "%I:%M%p", tm_info) == 0) {
            fprintf(stderr, "child: unable to format time\n");
            close(child_to_parent[1]);
            return 1;
        }

        const char *display_time = time_buf;

        if (display_time[0] == '0') {
            display_time++;
        }

        printf("Child: The time is %s, IT'S COMP2017 TIME!\n", display_time);
        fflush(stdout);

        if (write_string(child_to_parent[1], display_time) == -1) {
            perror("child write");
            close(child_to_parent[1]);
            return 1;
        }

        close(child_to_parent[1]);

        return 0;
    }

    /*
     * Parent process:
     * - prints and sends the question
     * - waits for the child's response
     * - prints thank you
     */

    close(parent_to_child[0]);   // parent does not read from parent_to_child
    close(child_to_parent[1]);   // parent does not write to child_to_parent

    const char *question = "Hi! Do you know what time it is?";

    printf("Parent: %s\n", question);
    fflush(stdout);

    if (write_string(parent_to_child[1], question) == -1) {
        perror("parent write");
        close(parent_to_child[1]);
        close(child_to_parent[0]);
        waitpid(pid, NULL, 0);
        return 1;
    }

    close(parent_to_child[1]);

    char response[BUF_SIZE];

    ssize_t nread = read(child_to_parent[0], response, sizeof(response) - 1);

    if (nread == -1) {
        perror("parent read");
        close(child_to_parent[0]);
        waitpid(pid, NULL, 0);
        return 1;
    }

    if (nread > 0) {
        response[nread] = '\0';
    }

    close(child_to_parent[0]);

    printf("Parent: Thank you!\n");

    waitpid(pid, NULL, 0);

    return 0;
}
