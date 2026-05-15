// 2026 S1C COMP2017 Week 11 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// A simple race condition demo.
// Compile with:
// gcc -std=c11 -Wall -Wextra -g -O1 -fsanitize=thread race.c -o race -pthread


#include <pthread.h>
#include <stdio.h>

#define NTHREADS 4
#define NINCREMENTS 100000

static int counter = 0;

static void *worker(void *arg) {
    (void)arg;

    for (int i = 0; i < NINCREMENTS; i++) {
        counter++;
    }

    return NULL;
}

int main(void) {
    pthread_t threads[NTHREADS];

    for (int i = 0; i < NTHREADS; i++) {
        pthread_create(&threads[i], NULL, worker, NULL);
    }

    for (int i = 0; i < NTHREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("counter = %d\n", counter);
    return 0;
}
