// 2026 S1C COMP2017 Week 9 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Parallel Search: Children processes for searching each single file.


#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s word file1 file2 ...\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *word = argv[1];
    int nfiles = argc - 2;

    pid_t *pids = calloc(nfiles, sizeof *pids);
    if (pids == NULL) {
        perror("calloc");
        return EXIT_FAILURE;
    }

    int spawned = 0;
    int failed = 0;

    for (int i = 0; i < nfiles; i++) {
        char *filename = argv[i + 2];

        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            failed = 1;
            break;
        }

        if (pid == 0) {
            execlp("grep", "grep", "-F", "-q", "--", word, filename, (char *)NULL);
            perror("execlp");
            _exit(127);
        }

        pids[spawned++] = pid;
    }

    for (int done = 0; done < spawned; done++) {
        int status;
        pid_t finished_pid = wait(&status);

        if (finished_pid == -1) {
            perror("wait");
            failed = 1;
            break;
        }

        int file_index = -1;

        for (int i = 0; i < spawned; i++) {
            if (pids[i] == finished_pid) {
                file_index = i;
                break;
            }
        }

        char *filename = argv[file_index + 2];

        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);

            if (code == 0) {
                printf("%s found\n", filename);
            } else if (code == 1) {
                printf("%s not found\n", filename);
            } else {
                printf("%s error\n", filename);
                failed = 1;
            }
        } else {
            printf("%s error\n", filename);
            failed = 1;
        }
    }

    printf("process complete\n");

    free(pids);
    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
