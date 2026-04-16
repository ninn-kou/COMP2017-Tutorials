// 2026 S1C COMP2017 Week 7 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Generic HashMap: main.c, hash_utils.c, hash_utils.h, hashmap.c, hashmap.h


#ifndef HASHMAP_H
#define HASHMAP_H

#include <stddef.h>

struct hashmap_entry {
    void *key;
    void *value;
    struct hashmap_entry *next;
};

struct hashmap {
    struct hashmap_entry **buckets;
    size_t bucket_count;
    size_t count;

    size_t (*hash)(struct hashmap *, void *);
    void (*keydel)(void *);
    void (*valdel)(void *);
    int (*cmp)(void *, void *);
};

void hashmap_init(struct hashmap *map,
                  size_t (*hash)(struct hashmap *, void *),
                  void (*keydel)(void *),
                  void (*valdel)(void *),
                  int (*cmp)(void *, void *),
                  size_t size);

void hashmap_insert(struct hashmap *map, void *key, void *value);
void *hashmap_get(struct hashmap *map, void *key);
void *hashmap_get_entry(struct hashmap *map, void *key);
void *hashmap_remove_value(struct hashmap *map, void *key);
void *hashmap_remove_entry(struct hashmap *map, void *key);
size_t hashmap_size(struct hashmap *map);
void hashmap_destroy(struct hashmap *map);

#endif
