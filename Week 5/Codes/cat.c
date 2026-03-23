// 2026 S1C COMP2017 Week 5 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// My `cat` command. It reads from stdin if no argument passed, otherwise it will catch file contents.
// Multiple files supported.


#include <stdio.h>

#define BUFFER_SIZE 4096

int cat_stdin(void) {
    char buffer[BUFFER_SIZE];

    while (fgets(buffer, sizeof buffer, stdin) != NULL) {
        if (fprintf(stdout, "%s", buffer) < 0) {
            return 1;
        }
    }

    return 0;
}

int cat_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Could not open %s\n", filename);
        return 1;
    }

    unsigned char buffer[BUFFER_SIZE];
    size_t nread;

    while ((nread = fread(buffer, sizeof(unsigned char), sizeof buffer, fp)) > 0) {
        for (size_t i = 0; i < nread; i++) {
            if (fprintf(stdout, "%c", buffer[i]) < 0) {
                fclose(fp);
                return 1;
            }
        }
    }

    if (fclose(fp) == EOF) {
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    int status = 0;

    if (argc == 1) {
        return cat_stdin();
    }

    for (int i = 1; i < argc; i++) {
        if (cat_file(argv[i]) != 0) {
            status = 1;
        }
    }

    return status;
}
