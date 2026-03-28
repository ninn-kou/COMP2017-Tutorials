// 2026 S1C COMP2017 Week 5 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Count bytes/words/lines for file(s).


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 4096

struct Counts {
    unsigned long lines;
    unsigned long words;
    unsigned long bytes;
};

static int count_stream(FILE *fp, struct Counts *counts) {
    unsigned char buffer[BUFFER_SIZE];
    size_t nread;
    int in_word = 0;

    while ((nread = fread(buffer, 1, sizeof buffer, fp)) > 0) {
        counts->bytes += (unsigned long) nread;

        for (size_t i = 0; i < nread; i++) {
            unsigned char ch = buffer[i];

            if (ch == '\n') {
                counts->lines++;
            }

            if (isspace((unsigned char) ch)) {
                in_word = 0;
            } else if (!in_word) {
                in_word = 1;
                counts->words++;
            }
        }
    }

    return ferror(fp) ? -1 : 0;
}

static void add_counts(struct Counts *total, const struct Counts *counts) {
    total->lines += counts->lines;
    total->words += counts->words;
    total->bytes += counts->bytes;
}

static int print_counts(const struct Counts *counts, const char *name) {
    if (name == NULL) {
        return printf("%lu %lu %lu\n",
                      counts->lines,
                      counts->words,
                      counts->bytes) < 0 ? -1 : 0;
    }

    return printf("%lu %lu %lu %s\n",
                  counts->lines,
                  counts->words,
                  counts->bytes,
                  name) < 0 ? -1 : 0;
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        struct Counts counts = {0, 0, 0};

        if (count_stream(stdin, &counts) != 0) {
            perror("stdin");
            return EXIT_FAILURE;
        }

        if (print_counts(&counts, NULL) != 0) {
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

    struct Counts total = {0, 0, 0};
    int processed = 0;
    int exit_status = EXIT_SUCCESS;

    for (int i = 1; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");
        if (fp == NULL) {
            perror(argv[i]);
            exit_status = EXIT_FAILURE;
            continue;
        }

        struct Counts counts = {0, 0, 0};

        if (count_stream(fp, &counts) != 0) {
            perror(argv[i]);
            fclose(fp);
            exit_status = EXIT_FAILURE;
            continue;
        }

        fclose(fp);

        if (print_counts(&counts, argv[i]) != 0) {
            return EXIT_FAILURE;
        }

        add_counts(&total, &counts);
        processed++;
    }

    if (argc > 2 && processed > 0) {
        if (print_counts(&total, "total") != 0) {
            return EXIT_FAILURE;
        }
    }

    return exit_status;
}
