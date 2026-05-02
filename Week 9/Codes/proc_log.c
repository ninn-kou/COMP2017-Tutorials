// 2026 S1C COMP2017 Week 9 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Logger: Process Output Logger with `popen()` and `pclose()`


#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 4096

static void print_usage(const char *program_name) {
    fprintf(stderr,
            "Usage: %s --cmd=\"<command>\" --out=<output-file>\n",
            program_name);
}

static const char *get_arg_value(const char *arg, const char *prefix) {
    size_t prefix_len = strlen(prefix);

    if (strncmp(arg, prefix, prefix_len) == 0) {
        return arg + prefix_len;
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    const char *cmd = NULL;
    const char *out_path = NULL;

    for (int i = 1; i < argc; i++) {
        const char *cmd_value = get_arg_value(argv[i], "--cmd=");
        const char *out_value = get_arg_value(argv[i], "--out=");

        if (cmd_value != NULL) {
            cmd = cmd_value;
        } else if (out_value != NULL) {
            out_path = out_value;
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (cmd == NULL || cmd[0] == '\0') {
        fprintf(stderr, "Missing or empty --cmd argument\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (out_path == NULL || out_path[0] == '\0') {
        fprintf(stderr, "Missing or empty --out argument\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    FILE *out_file = fopen(out_path, "wb");

    if (out_file == NULL) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    FILE *cmd_stream = popen(cmd, "r");

    if (cmd_stream == NULL) {
        perror("popen");
        fclose(out_file);
        return EXIT_FAILURE;
    }

    unsigned char buffer[BUFFER_SIZE];
    int copy_failed = 0;

    while (1) {
        size_t nread = fread(buffer, 1, sizeof buffer, cmd_stream);

        if (nread > 0) {
            size_t nwritten = fwrite(buffer, 1, nread, out_file);

            if (nwritten != nread) {
                perror("fwrite");
                copy_failed = 1;
                break;
            }
        }

        if (nread < sizeof buffer) {
            if (feof(cmd_stream)) {
                break;
            }

            if (ferror(cmd_stream)) {
                perror("fread");
                copy_failed = 1;
                break;
            }
        }
    }

    if (fclose(out_file) == EOF) {
        perror("fclose");
        copy_failed = 1;
    }

    int status = pclose(cmd_stream);

    if (status == -1) {
        perror("pclose");
        return EXIT_FAILURE;
    }

    if (copy_failed) {
        return EXIT_FAILURE;
    }

    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);

        if (exit_code != 0) {
            fprintf(stderr, "Command exited with status %d\n", exit_code);
            return exit_code;
        }
    } else if (WIFSIGNALED(status)) {
        fprintf(stderr,
                "Command was terminated by signal %d\n",
                WTERMSIG(status));
        return EXIT_FAILURE;
    } else {
        fprintf(stderr, "Command ended unexpectedly\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
