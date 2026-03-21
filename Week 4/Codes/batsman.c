// 2026 S1C COMP2017 Week 4 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// https://edstem.org/au/courses/31567/lessons/99435/slides/682664


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BATSMEN 11
#define MAX_NAME_LEN 100

struct cricketer {
    char *first_name;
    char *last_name;
    int score;
};

char *copy_name(const char *src) {
    size_t len = strlen(src) + 1;
    char *dst = malloc(len);

    if (dst != NULL) {
        strcpy(dst, src);
    }

    return dst;
}

void free_team(struct cricketer team[], int count) {
    for (int i = 0; i < count; i++) {
        free(team[i].first_name);
        free(team[i].last_name);
    }
}

void print_cricketer(const struct cricketer *player, int number) {
    printf("%d. %c. %s: ", number, player->first_name[0], player->last_name);

    if (player->score == 0) {
        printf("Duck\n");
    } else {
        printf("%d\n", player->score);
    }
}

int main(void) {
    struct cricketer team[MAX_BATSMEN] = {0};
    int total_runs = 0;

    for (int i = 0; i < MAX_BATSMEN; i++) {
        char first[MAX_NAME_LEN + 1];
        char last[MAX_NAME_LEN + 1];
        int score;

        printf("Enter Name and Score for batter %d: ", i + 1);

        if (scanf("%100s %100s %d", first, last, &score) != 3) {
            fprintf(stderr, "Invalid input\n");
            free_team(team, MAX_BATSMEN);
            return 1;
        }

        team[i].first_name = copy_name(first);
        team[i].last_name = copy_name(last);

        if (team[i].first_name == NULL || team[i].last_name == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            free_team(team, MAX_BATSMEN);
            return 1;
        }

        team[i].score = score;
        total_runs += score;
    }

    printf("\n");

    for (int i = 0; i < MAX_BATSMEN; i++) {
        print_cricketer(&team[i], i + 1);
    }

    printf("\nTotal runs: %d\n", total_runs);

    free_team(team, MAX_BATSMEN);
    return 0;
}
