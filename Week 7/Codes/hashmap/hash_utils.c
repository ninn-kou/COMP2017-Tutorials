// 2026 S1C COMP2017 Week 7 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Generic HashMap: main.c, hash_utils.c, hash_utils.h, hashmap.c, hashmap.h


#include "hash_utils.h"

#include <stdlib.h>
#include <string.h>

size_t djb2_hash(struct hashmap *map, void *key) {
    (void)map;

    const unsigned char *str = (const unsigned char *)key;
    size_t hash = 5381;
    int c;

    while ((c = *str++) != 0) {
        hash = ((hash << 5) + hash) + (size_t)c;
    }

    return hash;
}

int string_cmp(void *a, void *b) {
    return strcmp((const char *)a, (const char *)b);
}

void free_ptr(void *p) {
    free(p);
}
