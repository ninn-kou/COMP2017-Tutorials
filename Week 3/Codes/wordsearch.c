// 2026 S1C COMP2017 Week 3 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Search a word in a file, and return the line which contains string matched.


#include <stdio.h>

#define MAX_LINE 1024

int string_compare(const char s1[], const char s2[]) {
    int i = 0;

    while (s1[i] == s2[i]) {
        if (s1[i] == '\0') {
            return 0;   /* equal */
        }
        i++;
    }

    return s1[i] - s2[i];
}

int string_length(const char s[]) {
    int i = 0;

    while (s[i] != '\0') {
        i++;
    }

    return i;
}

void trim_newline(char s[]) {
    int i = 0;

    while (s[i] != '\0') {
        if (s[i] == '\n') {
            s[i] = '\0';
            return;
        }
        i++;
    }
}

void copy_part(const char src[], int start, int len, char dest[]) {
    int i;

    for (i = 0; i < len && src[start + i] != '\0'; i++) {
        dest[i] = src[start + i];
    }

    dest[i] = '\0';
}

int contains_word(const char line[], const char word[]) {
    int i;
    int word_len = string_length(word);
    char part[MAX_LINE];

    if (word_len == 0) {
        return 1;
    }

    if (word_len >= MAX_LINE) {
        return 0;
    }

    for (i = 0; line[i] != '\0'; i++) {
        copy_part(line, i, word_len, part);

        if (string_compare(part, word) == 0) {
            return 1;
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    char line[MAX_LINE];
    char *word;

    if (argc != 2) {
        printf("Usage: %s word\n", argv[0]);
        return 1;
    }

    word = argv[1];

    while (fgets(line, sizeof line, stdin) != NULL) {
        trim_newline(line);

        if (contains_word(line, word)) {
            printf("Found: %s\n", line);
        }
    }

    return 0;
}
