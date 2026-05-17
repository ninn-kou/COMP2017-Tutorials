// thread_demo.c — 5 getters + 1 putter hammering the DNS cache
// YOU DO NOT NEED TO MODIFY THIS FILE
// JUST HERE TO TEST YOUR CODE
// Tries to creates NUM_ENTRIES as NUM_GETTERS keep asking for more...

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dns_cache.h"

#define NUM_GETTERS 5
#define NUM_ENTRIES 4

static dns_cache_t cache;

static const char *domains[NUM_ENTRIES] = {
    "google.com",
    "github.com",
    "anthropic.com",
    "unsw.edu.au",
};

static const char *ips[NUM_ENTRIES] = {
    "142.250.70.46",
    "140.82.121.4",
    "160.79.104.10",
    "129.94.242.21",
};

static void *getter_main(void *arg) {
    int id = *(int *)arg;

    for (int i = 0; i < NUM_ENTRIES; i++) {
        const char *domain = domains[i];

        while (1) {
            char *ip = cache_get(&cache, domain);
            if (ip != NULL) {
                printf("[getter %d] got %s -> %s\n", id, domain, ip);
                free(ip);
                break;
            }
            printf("[getter %d] miss on %s, retrying...\n", id, domain);
            sleep(1);
        }

        sleep(1);
    }
    return NULL;
}

static void *putter_main(void *arg) {
    (void)arg;

    sleep(2);
    for (int i = 0; i < NUM_ENTRIES; i++) {
        printf("[putter] inserting %s\n", domains[i]);
        cache_put(&cache, domains[i], ips[i]);
        sleep(2);
    }
    return NULL;
}

int main(void) {
    cache_init(&cache);

    pthread_t getters[NUM_GETTERS];
    int       ids[NUM_GETTERS];
    pthread_t putter;

    for (int i = 0; i < NUM_GETTERS; i++) {
        ids[i] = i;
        pthread_create(&getters[i], NULL, getter_main, &ids[i]);
    }
    pthread_create(&putter, NULL, putter_main, NULL);

    for (int i = 0; i < NUM_GETTERS; i++) {
        pthread_join(getters[i], NULL);
    }
    pthread_join(putter, NULL);

    cache_destroy(&cache);
    return 0;
}
