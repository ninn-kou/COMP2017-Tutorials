// 2026 S1C COMP2017 Week 4 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// 2D Substring.


#include <stdio.h>

// Define the grid size
#define ROWS 10
#define COLS 10

// Function prototypes
void search_horizontal(const char grid[ROWS][COLS], const char *str);
void search_vertical(const char grid[ROWS][COLS], const char *str);
void search_diagonal(const char grid[ROWS][COLS], const char *str);

/*
 * Compare str against the grid starting at (row, col),
 * moving by (row_step, col_step) each time.
 *
 * Returns 1 if the whole string matches, 0 otherwise.
 */
static int match_direction(const char grid[ROWS][COLS],
                           const char *str,
                           int row, int col,
                           int row_step, int col_step) {
    int i = 0;

    while (str[i] != '\0' && grid[row][col] == str[i]) {
        i++;
        row += row_step;
        col += col_step;
    }

    return str[i] == '\0';
}

void search_horizontal(const char grid[ROWS][COLS], const char *str) {
    int row, col;

    if (str[0] == '\0') {
        return;
    }

    for (row = 1; row < ROWS - 1; row++) {
        for (col = 1; col < COLS - 1; col++) {
            if (match_direction(grid, str, row, col, 0, 1)) {
                printf("Found horizontal match for \"%s\": row %d, column %d\n",
                       str, row, col);
            }
        }
    }
}

void search_vertical(const char grid[ROWS][COLS], const char *str) {
    int row, col;

    if (str[0] == '\0') {
        return;
    }

    for (row = 1; row < ROWS - 1; row++) {
        for (col = 1; col < COLS - 1; col++) {
            if (match_direction(grid, str, row, col, 1, 0)) {
                printf("Found vertical match for \"%s\": row %d, column %d\n",
                       str, row, col);
            }
        }
    }
}

void search_diagonal(const char grid[ROWS][COLS], const char *str) {
    int row, col;

    if (str[0] == '\0') {
        return;
    }

    for (row = 1; row < ROWS - 1; row++) {
        for (col = 1; col < COLS - 1; col++) {
            if (match_direction(grid, str, row, col, 1, 1)) {
                printf("Found diagonal match for \"%s\": row %d, column %d\n",
                       str, row, col);
            }
        }
    }
}

int main(void) {
    const char grid[ROWS][COLS] = {
        {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'},
        {'\0', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', '\0'},
        {'\0', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', '\0'},
        {'\0', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', '\0'},
        {'\0', 'y', 'z', 'a', 'b', 'c', 'd', 'e', 'f', '\0'},
        {'\0', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', '\0'},
        {'\0', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', '\0'},
        {'\0', 'w', 'x', 'y', 'z', 'a', 'b', 'c', 'd', '\0'},
        {'\0', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', '\0'},
        {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}
    };

    search_horizontal(grid, "abc");
    search_vertical(grid, "emu");
    search_diagonal(grid, "ajsb");
    search_horizontal(grid, "cat");   // no output
    search_horizontal(grid, "vwx");

    return 0;
}
