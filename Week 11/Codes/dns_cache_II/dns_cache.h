// 2026 S1C COMP2017 Week 11 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// DNS Cache II: dns_cache.h, dns_cache.c, thread_pool.c
// This file was modified when debugging.


#ifndef DNS_CACHE_H
#define DNS_CACHE_H

#include <pthread.h>

typedef struct dns_entry {
    char *domain;
    char *ip;
    struct dns_entry *next;
} dns_entry_t;

typedef struct dns_cache {
    dns_entry_t *head;
    pthread_rwlock_t lock;
} dns_cache_t;

void cache_init(dns_cache_t *cache);
void cache_destroy(dns_cache_t *cache);

char *cache_get(dns_cache_t *cache, const char *domain);
void cache_put(dns_cache_t *cache, const char *domain, const char *ip);

#endif
