// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// island_b.c
// 2 March 2026


#include <stdio.h>

int secret_location_gold = 10;

int main() {
    int wallet = 0;
    int* map_to_secret = &secret_location_gold;

    // Steal the gold without accessing secret_location_gold directly.
    wallet = *map_to_secret;
    *map_to_secret = 0;

    // Print how much gold is left on the island
    printf("There are %d gold pots on the island\n", *map_to_secret);
    printf("There are %d gold pots in my wallet\n", wallet);
}
