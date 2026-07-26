#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static pthread_mutex_t cache_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t journal_lock = PTHREAD_MUTEX_INITIALIZER;

struct storage_state {
    int cached_pages;
    int journal_entries;
};

static struct storage_state state = {
    .cached_pages = 1,
    .journal_entries = 1
};

static void update_cache(void *page) {
    (void) page;
    state.cached_pages++;
}

static void append_journal(void *page) {
    (void) page;
    state.journal_entries++;
}

static void prepare_flush(void *page) {
    (void) page;
}

static void evict_page(void *page) {
    (void) page;
    if (state.cached_pages > 0) {
        state.cached_pages--;
    }
    if (state.journal_entries > 0) {
        state.journal_entries--;
    }
}

static void lock_cache_then_journal(void) {
    pthread_mutex_lock(&cache_lock);
    pthread_mutex_lock(&journal_lock);
}

static void unlock_journal_then_cache(void) {
    pthread_mutex_unlock(&journal_lock);
    pthread_mutex_unlock(&cache_lock);
}

static void *store_page(void *argument) {
    pthread_mutex_lock(&cache_lock);
    update_cache(argument);
    pthread_mutex_lock(&journal_lock);
    append_journal(argument);
    unlock_journal_then_cache();
    return NULL;
}

static void *flush_page(void *argument) {
    lock_cache_then_journal();
    prepare_flush(argument);
    evict_page(argument);
    unlock_journal_then_cache();
    return NULL;
}

int main(void) {
    pthread_t store_thread;
    pthread_t flush_thread;

    int error = pthread_create(&store_thread, NULL, store_page, NULL);
    if (error != 0) {
        fprintf(stderr, "pthread_create: %s\n", strerror(error));
        return EXIT_FAILURE;
    }

    error = pthread_create(&flush_thread, NULL, flush_page, NULL);
    if (error != 0) {
        fprintf(stderr, "pthread_create: %s\n", strerror(error));
        pthread_join(store_thread, NULL);
        return EXIT_FAILURE;
    }

    int join_failed = 0;
    error = pthread_join(store_thread, NULL);
    if (error != 0) {
        fprintf(stderr, "pthread_join: %s\n", strerror(error));
        join_failed = 1;
    }
    error = pthread_join(flush_thread, NULL);
    if (error != 0) {
        fprintf(stderr, "pthread_join: %s\n", strerror(error));
        join_failed = 1;
    }
    if (join_failed) {
        return EXIT_FAILURE;
    }

    printf("cached_pages=%d journal_entries=%d\n",
           state.cached_pages, state.journal_entries);

    pthread_mutex_destroy(&journal_lock);
    pthread_mutex_destroy(&cache_lock);
    return EXIT_SUCCESS;
}
