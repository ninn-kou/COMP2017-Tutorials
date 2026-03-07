// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// island_c.c
// 2 March 2026


#include <stdio.h>

int secret_location_gold = 10;
int* secret_map = &secret_location_gold;

int main() {
    int wallet = 0;
    int** map_to_secret_map = &secret_map;

    // Steal the gold without accessing secret_location_gold or secret_map directly.
    wallet = **map_to_secret_map;
    **map_to_secret_map = 0;

    // Print how much gold is left on the island
    printf("There are %d gold pots on the island\n", **map_to_secret_map);
    printf("There are %d gold pots in my wallet\n", wallet);
}
