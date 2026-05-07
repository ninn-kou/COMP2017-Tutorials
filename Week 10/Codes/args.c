// 2026 S1C COMP2017 Week 10 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Hello from threads with arguments.


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NTHREADS 10

static void *routine(void *args) {
    int id = *(int *)args;

    printf("Hello From Thread %d!\n", id);

    return NULL;
}

int main(void) {
    pthread_t threads[NTHREADS];
    int ids[NTHREADS];

    for (int i = 0; i < NTHREADS; i++) {
        ids[i] = i;

        int err = pthread_create(&threads[i], NULL, routine, &ids[i]);

        if (err != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(err));
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < NTHREADS; i++) {
        int err = pthread_join(threads[i], NULL);

        if (err != 0) {
            fprintf(stderr, "pthread_join: %s\n", strerror(err));
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
