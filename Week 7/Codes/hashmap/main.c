// 2026 S1C COMP2017 Week 7 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Generic HashMap: main.c, hash_utils.c, hash_utils.h, hashmap.c, hashmap.h


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash_utils.h"
#include "hashmap.h"

static char *demo_strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (copy != NULL) {
        memcpy(copy, s, len);
    }
    return copy;
}

int main(void) {
    struct hashmap map;
    hashmap_init(&map, djb2_hash, free_ptr, free_ptr, string_cmp, 8);

    hashmap_insert(&map, demo_strdup("apple"), demo_strdup("red"));
    hashmap_insert(&map, demo_strdup("banana"), demo_strdup("yellow"));
    hashmap_insert(&map, demo_strdup("grape"), demo_strdup("purple"));

    printf("banana -> %s\n", (char *)hashmap_get(&map, "banana"));

    char *removed_value = hashmap_remove_value(&map, "apple");
    if (removed_value != NULL) {
        printf("Removed value: %s\n", removed_value);
        free(removed_value);
    }

    struct hashmap_entry *removed_entry = hashmap_remove_entry(&map, "grape");
    if (removed_entry != NULL) {
        printf("Removed entry: %s -> %s\n",
               (char *)removed_entry->key,
               (char *)removed_entry->value);

        free(removed_entry->key);
        free(removed_entry->value);
        free(removed_entry);
    }

    printf("Current size: %zu\n", hashmap_size(&map));

    hashmap_destroy(&map);
    return 0;
}
