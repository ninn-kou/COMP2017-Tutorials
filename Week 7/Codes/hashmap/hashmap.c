// 2026 S1C COMP2017 Week 7 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Generic HashMap: main.c, hash_utils.c, hash_utils.h, hashmap.c, hashmap.h


#include "hashmap.h"

#include <stdio.h>
#include <stdlib.h>

static size_t bucket_index(struct hashmap *map, void *key) {
    return map->hash(map, key) % map->bucket_count;
}

static int hashmap_resize(struct hashmap *map, size_t new_bucket_count) {
    struct hashmap_entry **new_buckets =
        calloc(new_bucket_count, sizeof *new_buckets);
    if (new_buckets == NULL) {
        return 0;
    }

    for (size_t i = 0; i < map->bucket_count; i++) {
        struct hashmap_entry *entry = map->buckets[i];

        while (entry != NULL) {
            struct hashmap_entry *next = entry->next;
            size_t idx = map->hash(map, entry->key) % new_bucket_count;
            entry->next = new_buckets[idx];
            new_buckets[idx] = entry;
            entry = next;
        }
    }

    free(map->buckets);
    map->buckets = new_buckets;
    map->bucket_count = new_bucket_count;
    return 1;
}

void hashmap_init(struct hashmap *map,
                  size_t (*hash)(struct hashmap *, void *),
                  void (*keydel)(void *),
                  void (*valdel)(void *),
                  int (*cmp)(void *, void *),
                  size_t size) {
    map->bucket_count = (size == 0) ? 8 : size;
    map->count = 0;
    map->hash = hash;
    map->keydel = keydel;
    map->valdel = valdel;
    map->cmp = cmp;
    map->buckets = calloc(map->bucket_count, sizeof *map->buckets);

    if (map->buckets == NULL) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
}

void hashmap_insert(struct hashmap *map, void *key, void *value) {
    if ((map->count + 1) * 4 > map->bucket_count * 3) {
        if (!hashmap_resize(map, map->bucket_count * 2)) {
            perror("hashmap_resize");
            exit(EXIT_FAILURE);
        }
    }

    size_t idx = bucket_index(map, key);

    for (struct hashmap_entry *cur = map->buckets[idx];
         cur != NULL;
         cur = cur->next) {
        if (map->cmp(cur->key, key) == 0) {
            if (map->keydel != NULL && cur->key != key) {
                map->keydel(key);
            }
            if (map->valdel != NULL) {
                map->valdel(cur->value);
            }
            cur->value = value;
            return;
        }
    }

    struct hashmap_entry *entry = malloc(sizeof *entry);
    if (entry == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    entry->key = key;
    entry->value = value;
    entry->next = map->buckets[idx];
    map->buckets[idx] = entry;
    map->count++;
}

void *hashmap_get_entry(struct hashmap *map, void *key) {
    size_t idx = bucket_index(map, key);

    for (struct hashmap_entry *cur = map->buckets[idx];
         cur != NULL;
         cur = cur->next) {
        if (map->cmp(cur->key, key) == 0) {
            return cur;
        }
    }

    return NULL;
}

void *hashmap_get(struct hashmap *map, void *key) {
    struct hashmap_entry *entry = hashmap_get_entry(map, key);
    return entry ? entry->value : NULL;
}

void *hashmap_remove_value(struct hashmap *map, void *key) {
    size_t idx = bucket_index(map, key);
    struct hashmap_entry *prev = NULL;
    struct hashmap_entry *cur = map->buckets[idx];

    while (cur != NULL) {
        if (map->cmp(cur->key, key) == 0) {
            if (prev == NULL) {
                map->buckets[idx] = cur->next;
            } else {
                prev->next = cur->next;
            }

            if (map->keydel != NULL) {
                map->keydel(cur->key);
            }

            void *value = cur->value;
            free(cur);
            map->count--;
            return value;
        }

        prev = cur;
        cur = cur->next;
    }

    return NULL;
}

void *hashmap_remove_entry(struct hashmap *map, void *key) {
    size_t idx = bucket_index(map, key);
    struct hashmap_entry *prev = NULL;
    struct hashmap_entry *cur = map->buckets[idx];

    while (cur != NULL) {
        if (map->cmp(cur->key, key) == 0) {
            if (prev == NULL) {
                map->buckets[idx] = cur->next;
            } else {
                prev->next = cur->next;
            }

            cur->next = NULL;
            map->count--;
            return cur;
        }

        prev = cur;
        cur = cur->next;
    }

    return NULL;
}

size_t hashmap_size(struct hashmap *map) {
    return map->count;
}

void hashmap_destroy(struct hashmap *map) {
    if (map == NULL) {
        return;
    }

    for (size_t i = 0; i < map->bucket_count; i++) {
        struct hashmap_entry *cur = map->buckets[i];

        while (cur != NULL) {
            struct hashmap_entry *next = cur->next;

            if (map->keydel != NULL) {
                map->keydel(cur->key);
            }
            if (map->valdel != NULL) {
                map->valdel(cur->value);
            }

            free(cur);
            cur = next;
        }
    }

    free(map->buckets);
    map->buckets = NULL;
    map->bucket_count = 0;
    map->count = 0;
}
