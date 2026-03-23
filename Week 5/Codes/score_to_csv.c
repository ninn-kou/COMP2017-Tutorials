// 2026 S1C COMP2017 Week 5 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Using the struct batsman definition, output the fields of an array of struct batsman to a comma separated value file (CSV).


#include <stdio.h>

// The struct batsman definition.
struct batsman {
    char first_name[20];
    char last_name[20];
    int score;
};

int output_scores(struct batsman* batter, int no_batsmen, const char* filename);

int main(void) {
    struct batsman team[] = {
        {"Cameron", "Bancroft", 40},
        {"Mitchell", "Marsh", 67},
        {"David", "Warner", 59},
        {"Ben", "McDermott", 0},
        {"Cameron", "White", 78},
        {"Usman", "Khawja", 54}
    };

    int n = sizeof(team) / sizeof(team[0]);

    if (output_scores(team, n, "scores.csv") != 0) {
        printf("Could not write scores.csv\n");
        return 1;
    }

    printf("scores.csv written successfully\n");
    return 0;
}

int output_scores(struct batsman* batter, int no_batsmen, const char* filename) {
    if (filename == NULL || no_batsmen < 0 || (batter == NULL && no_batsmen > 0)) {
        return -1;
    }

    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        return -1;
    }

    for (int i = 0; i < no_batsmen; i++) {
        int result;

        if (batter[i].score == 0) {
            result = fprintf(fp, "%s,%s,Duck\n",
                             batter[i].first_name,
                             batter[i].last_name);
        } else {
            result = fprintf(fp, "%s,%s,%d\n",
                             batter[i].first_name,
                             batter[i].last_name,
                             batter[i].score);
        }

        if (result < 0) {
            fclose(fp);
            return -1;
        }
    }

    if (fclose(fp) == EOF) {
        return -1;
    }

    return 0;
}
