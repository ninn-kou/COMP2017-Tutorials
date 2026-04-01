// 2026 S1C COMP2017 Week 6 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Read from STDIN in a non-blocking fashion.


#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define BUFFER_SIZE 256

int main(void) {
    char buffer[BUFFER_SIZE];
    int seconds = 0;
    int exit_status = 0;

    /* Get current stdin flags */
    int old_flags = fcntl(STDIN_FILENO, F_GETFL);
    if (old_flags == -1) {
        perror("fcntl(F_GETFL)");
        return 1;
    }

    /* Set stdin to nonblocking, while preserving existing flags */
    if (fcntl(STDIN_FILENO, F_SETFL, old_flags | O_NONBLOCK) == -1) {
        perror("fcntl(F_SETFL)");
        return 1;
    }

    while (1) {
        ssize_t bytes = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);

        if (bytes == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                seconds++;
                printf("Waiting for %d seconds...\n", seconds);
                fflush(stdout);
                sleep(1);
                continue;
            } else {
                perror("read");
                exit_status = 1;
                break;
            }
        } else if (bytes == 0) {
            /* EOF, e.g. Ctrl-D on a terminal */
            printf("Exiting...\n");
            break;
        } else {
            buffer[bytes] = '\0';
            buffer[strcspn(buffer, "\n")] = '\0';

            if (strcmp(buffer, "exit") == 0) {
                printf("Exiting...\n");
                break;
            }

            printf("You typed: %s\n", buffer);
            fflush(stdout);
            seconds = 0;
        }
    }

    /* Restore original flags before exiting */
    if (fcntl(STDIN_FILENO, F_SETFL, old_flags) == -1) {
        perror("fcntl(restore)");
        return 1;
    }

    return exit_status;
}
