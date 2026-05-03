// 2026 S1C COMP2017 Week 9 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Bash Simulator: sash.c, commands.c, commands.h


#define _POSIX_C_SOURCE 200809L

#include "commands.h"

#define WORD_DELIMS " \t\r\n"

/*
 * Counts how many times the characters in delim occur in str.
 * Returns -1 on error.
 */
int count_occurences(char *str, char *delims) {
    if (str == NULL || delims == NULL) {
        return -1;
    }

    int count = 0;

    for (int i = 0; str[i] != '\0'; i++) {
        if (strchr(delims, str[i]) != NULL) {
            count++;
        }
    }

    return count;
}

/*
 * Breaks a string into an array of strings separated by delims.
 * Edits the string in place.
 * The returned array is malloced and NULL-terminated.
 */
char **deconstruct_sentence(char *str, char *delims) {
    int n = count_occurences(str, delims);

    if (n < 0) {
        return NULL;
    }

    /*
     * n + 2 because:
     * - each delimiter can create one extra token
     * - we need one final NULL terminator
     */
    char **array = malloc(sizeof(char *) * (n + 2));

    if (array == NULL) {
        perror("malloc failed");
        return NULL;
    }

    memset(array, 0, sizeof(char *) * (n + 2));

    int i = 0;
    array[i] = strtok(str, delims);

    while (array[i] != NULL) {
        i++;
        array[i] = strtok(NULL, delims);
    }

    return array;
}

/*
 * Breaks str into a NULL-terminated array of commands.
 * Each command is a NULL-terminated argv array.
 *
 * The original string is edited in place.
 */
char ***deconstruct_command(char *str) {
    int n = count_occurences(str, "|");

    if (n < 0) {
        return NULL;
    }

    /*
     * n + 2 because:
     * - n pipes means n + 1 commands
     * - we need one final NULL terminator
     */
    char ***array = malloc(sizeof(char **) * (n + 2));

    if (array == NULL) {
        perror("malloc failed");
        return NULL;
    }

    memset(array, 0, sizeof(char **) * (n + 2));

    /*
     * First split into command strings using '|'.
     */
    char **temp_array = deconstruct_sentence(str, "|");

    if (temp_array == NULL) {
        perror("failed to split by |");
        free(array);
        return NULL;
    }

    /*
     * Then split each command into words.
     */
    for (int i = 0; temp_array[i] != NULL; i++) {
        array[i] = deconstruct_sentence(temp_array[i], WORD_DELIMS);

        if (array[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(array[j]);
            }

            free(temp_array);
            free(array);

            return NULL;
        }
    }

    free(temp_array);

    return array;
}

/*
 * Should print out all the words on new lines.
 */
void test_deconstruct_sentence(void) {
    /*
     * We use strdup because we need to be able to edit the string,
     * which we cannot do for string literals.
     */
    char *str = strdup("Well I wonder if I'll get all these words");

    if (str == NULL) {
        perror("strdup");
        return;
    }

    char **decon = deconstruct_sentence(str, WORD_DELIMS);

    if (decon == NULL) {
        free(str);
        return;
    }

    int i = 0;

    while (decon[i] != NULL) {
        printf("%s\n", decon[i]);
        i++;
    }

    free(str);
    free(decon);
}

/*
 * Frees all the memory associated with a deconstructed command.
 * Does not free the root string, because the words point into it.
 */
void free_decon_command(char ***decon_cmd) {
    if (decon_cmd == NULL) {
        return;
    }

    int i = 0;

    while (decon_cmd[i] != NULL) {
        free(decon_cmd[i]);
        i++;
    }

    free(decon_cmd);
}

void test_deconstruct_command(void) {
    /*
     * We use strdup because we need to be able to edit the string,
     * which we cannot do for string literals.
     */
    char *str = strdup(
        "Well I wonder if I'll get all these words | and then these two"
    );

    if (str == NULL) {
        perror("strdup");
        return;
    }

    char ***decon = deconstruct_command(str);

    if (decon == NULL) {
        free(str);
        return;
    }

    int i = 0;

    while (decon[i] != NULL) {
        int j = 0;

        while (decon[i][j] != NULL) {
            printf("%s, ", decon[i][j]);
            j++;
        }

        printf("\nNew Command\n");
        i++;
    }

    free(str);
    free_decon_command(decon);
}
