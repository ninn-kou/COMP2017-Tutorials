// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// island_a.c
// 2 March 2026


#include <stdio.h>

int main() {
    int gold_pots = 10;
    int wallet = 0;

    // Steal the gold
    wallet = gold_pots;
    gold_pots = 0;

    printf("There are %d gold pots on the island\n", gold_pots);
    printf("There are %d gold pots in my wallet\n", wallet);
}
