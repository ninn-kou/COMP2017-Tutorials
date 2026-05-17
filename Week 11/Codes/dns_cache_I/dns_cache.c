// 2026 S1C COMP2017 Week 11 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// DNS Cache I: dns_cache.h, dns_cache.c, thread_pool.c


#include "dns_cache.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void die_pthread(int err, const char *message) {
    if (err != 0) {
        fprintf(stderr, "%s: %s\n", message, strerror(err));
        exit(EXIT_FAILURE);
    }
}

static char *duplicate_string(const char *s) {
    size_t len = strlen(s) + 1;

    char *copy = malloc(len);
    if (copy == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    memcpy(copy, s, len);
    return copy;
}

void cache_init(dns_cache_t *cache) {
    cache->head = NULL;

    int err = pthread_mutex_init(&cache->lock, NULL);
    die_pthread(err, "pthread_mutex_init");
}

void cache_destroy(dns_cache_t *cache) {
    int err = pthread_mutex_lock(&cache->lock);
    die_pthread(err, "pthread_mutex_lock");

    dns_entry_t *curr = cache->head;

    while (curr != NULL) {
        dns_entry_t *next = curr->next;

        free(curr->domain);
        free(curr->ip);
        free(curr);

        curr = next;
    }

    cache->head = NULL;

    err = pthread_mutex_unlock(&cache->lock);
    die_pthread(err, "pthread_mutex_unlock");

    err = pthread_mutex_destroy(&cache->lock);
    die_pthread(err, "pthread_mutex_destroy");
}

char *cache_get(dns_cache_t *cache, const char *domain) {
    int err = pthread_mutex_lock(&cache->lock);
    die_pthread(err, "pthread_mutex_lock");

    for (dns_entry_t *curr = cache->head; curr != NULL; curr = curr->next) {
        if (strcmp(curr->domain, domain) == 0) {
            char *ip_copy = duplicate_string(curr->ip);

            err = pthread_mutex_unlock(&cache->lock);
            die_pthread(err, "pthread_mutex_unlock");

            return ip_copy;
        }
    }

    err = pthread_mutex_unlock(&cache->lock);
    die_pthread(err, "pthread_mutex_unlock");

    return NULL;
}

void cache_put(dns_cache_t *cache, const char *domain, const char *ip) {
    dns_entry_t *new_entry = malloc(sizeof *new_entry);

    if (new_entry == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    char *domain_copy = duplicate_string(domain);
    char *ip_copy = duplicate_string(ip);

    int err = pthread_mutex_lock(&cache->lock);
    die_pthread(err, "pthread_mutex_lock");

    /*
     * If the domain already exists, update its IP address.
     */
    for (dns_entry_t *curr = cache->head; curr != NULL; curr = curr->next) {
        if (strcmp(curr->domain, domain) == 0) {
            free(curr->ip);
            curr->ip = ip_copy;

            err = pthread_mutex_unlock(&cache->lock);
            die_pthread(err, "pthread_mutex_unlock");

            free(domain_copy);
            free(new_entry);
            return;
        }
    }

    /*
     * Insert at the front of the list.
     * This is O(1), so the mutex is held for less time.
     */
    new_entry->domain = domain_copy;
    new_entry->ip = ip_copy;
    new_entry->next = cache->head;
    cache->head = new_entry;

    err = pthread_mutex_unlock(&cache->lock);
    die_pthread(err, "pthread_mutex_unlock");
}
