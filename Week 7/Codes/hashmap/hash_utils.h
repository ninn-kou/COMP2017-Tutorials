// 2026 S1C COMP2017 Week 7 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Generic HashMap: main.c, hash_utils.c, hash_utils.h, hashmap.c, hashmap.h


#ifndef HASH_UTILS_H
#define HASH_UTILS_H

#include <stddef.h>

struct hashmap;

size_t djb2_hash(struct hashmap *map, void *key);
int string_cmp(void *a, void *b);
void free_ptr(void *p);

#endif
