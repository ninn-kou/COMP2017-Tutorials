// 2026 S1C COMP2017 Week 12 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Demonstration of Condition Variables Usage


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

static int ready = 0;

static void *waiter(void *arg) {
    (void)arg;

    pthread_mutex_lock(&lock);

    while (!ready) {
        pthread_cond_wait(&cv, &lock);
    }

    printf("waiter: ready is true\n");

    pthread_mutex_unlock(&lock);
    return NULL;
}

static void *signaller(void *arg) {
    (void)arg;

    pthread_mutex_lock(&lock);

    ready = 1;
    pthread_cond_signal(&cv);

    pthread_mutex_unlock(&lock);
    return NULL;
}

int main(void) {
    pthread_t t1;
    pthread_t t2;

    pthread_create(&t1, NULL, waiter, NULL);
    pthread_create(&t2, NULL, signaller, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    pthread_cond_destroy(&cv);
    pthread_mutex_destroy(&lock);

    return EXIT_SUCCESS;
}
