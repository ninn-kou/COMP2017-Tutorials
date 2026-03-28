// 2026 S1C COMP2017 Week 5 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Replace a specific word in a document and generate a file for replaced word.


#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define MAX_WORD_LEN 256

static int write_word(FILE *out, const char *word,
                      const char *old_word, const char *new_word) {
    if (strcmp(word, old_word) == 0) {
        return fputs(new_word, out) == EOF ? 1 : 0;
    }

    return fputs(word, out) == EOF ? 1 : 0;
}

int replace_words(const char *input_name, const char *output_name,
                  const char *old_word, const char *new_word) {
    FILE *in = fopen(input_name, "r");
    if (in == NULL) {
        return 1;
    }

    FILE *out = fopen(output_name, "w");
    if (out == NULL) {
        fclose(in);
        return 1;
    }

    char word[MAX_WORD_LEN];
    int len = 0;
    int ch;
    int status = 0;

    while ((ch = fgetc(in)) != EOF) {
        if (isalpha((unsigned char) ch)) {
            if (len >= MAX_WORD_LEN - 1) {
                status = 1;
                break;
            }
            word[len++] = (char) ch;
        } else {
            if (len > 0) {
                word[len] = '\0';

                if (write_word(out, word, old_word, new_word) != 0) {
                    status = 1;
                    break;
                }

                len = 0;
            }

            if (fputc(ch, out) == EOF) {
                status = 1;
                break;
            }
        }
    }

    if (status == 0 && ferror(in)) {
        status = 1;
    }

    if (status == 0 && len > 0) {
        word[len] = '\0';

        if (write_word(out, word, old_word, new_word) != 0) {
            status = 1;
        }
    }

    if (fclose(in) == EOF) {
        status = 1;
    }

    if (fclose(out) == EOF) {
        status = 1;
    }

    return status == 0 ? 0 : 1;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s input output old_word new_word\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], argv[2]) == 0) {
        fprintf(stderr, "Input and output files must be different\n");
        return 1;
    }

    if (replace_words(argv[1], argv[2], argv[3], argv[4]) != 0) {
        fprintf(stderr, "Replacement failed\n");
        return 1;
    }

    return 0;
}
