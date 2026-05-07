// 2026 S1C COMP2017 Week 10 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Hello from threads.


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NTHREADS 5

static void *print_hello(void *arg) {
    (void)arg;

    printf("Hello World\n");
    return NULL;
}

int main(void) {
    pthread_t threads[NTHREADS];

    for (int i = 0; i < NTHREADS; i++) {
        int err = pthread_create(&threads[i], NULL, print_hello, NULL);

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
