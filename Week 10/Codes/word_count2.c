// 2026 S1C COMP2017 Week 10 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// A program that quickly searches for a given string in a large text file using memory mapping.
// 1. Accept two command-line arguments: the filename and the search string.
// 2. Use memory mapping to load the entire file into memory.
// 3. Perform a case-insensitive search for the given string.
// 4. Print the line number and content of each line containing the search string.
// 5. Display the total number of occurrences.


#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static int match_at(const char *text, const char *needle, size_t needle_len) {
    for (size_t i = 0; i < needle_len; i++) {
        unsigned char a = (unsigned char)text[i];
        unsigned char b = (unsigned char)needle[i];

        if (tolower(a) != tolower(b)) {
            return 0;
        }
    }

    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename> <search-string>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];
    const char *needle = argv[2];
    size_t needle_len = strlen(needle);

    if (needle_len == 0) {
        fprintf(stderr, "Search string must not be empty\n");
        return EXIT_FAILURE;
    }

    int fd = open(filename, O_RDONLY);

    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    struct stat st;

    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return EXIT_FAILURE;
    }

    if (st.st_size == 0) {
        printf("total occurrences: 0\n");
        close(fd);
        return EXIT_SUCCESS;
    }

    size_t file_size = (size_t)st.st_size;

    char *data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);

    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    close(fd);

    size_t total = 0;
    size_t line_no = 1;
    size_t i = 0;

    while (i < file_size) {
        size_t line_start = i;

        while (i < file_size && data[i] != '\n') {
            i++;
        }

        size_t line_end = i;
        int line_has_match = 0;

        if (line_end - line_start >= needle_len) {
            for (size_t pos = line_start;
                 pos + needle_len <= line_end;
                 pos++) {
                if (match_at(data + pos, needle, needle_len)) {
                    total++;
                    line_has_match = 1;
                }
            }
        }

        if (line_has_match) {
            printf("%zu: ", line_no);
            fwrite(data + line_start, 1, line_end - line_start, stdout);
            putchar('\n');
        }

        if (i < file_size && data[i] == '\n') {
            i++;
        }

        line_no++;
    }

    printf("total occurrences: %zu\n", total);

    if (munmap(data, file_size) == -1) {
        perror("munmap");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
