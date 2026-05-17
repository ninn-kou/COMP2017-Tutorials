// 2026 S1C COMP2017 Week 11 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
//
// Assume that there are N philosophers sitting at a round table.
// A single chopstick is placed between two adjacent philosophers.
// Every philosopher is either thinking or eating.
// However, a philosopher needs both chopsticks (to the left and to the right) to start eating.
// They are not allowed to acquire chopsticks that are not immediately adjacent to them.
// Complete the following program so that each philosopher is able to eat.


#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define THINKERS 5
#define MEALS_PER_THINKER 3

static pthread_mutex_t chopsticks[THINKERS];

static void check_pthread(int err, const char *message) {
    if (err != 0) {
        fprintf(stderr, "%s: %s\n", message, strerror(err));
        exit(EXIT_FAILURE);
    }
}

void *dine(void *arg) {
    const unsigned id = *((unsigned *)arg);

    unsigned left = id;
    unsigned right = (id + 1) % THINKERS;

    unsigned first = left < right ? left : right;
    unsigned second = left < right ? right : left;

    for (int meal = 0; meal < MEALS_PER_THINKER; meal++) {
        printf("Philosopher %u is thinking\n", id);
        sleep(1);

        check_pthread(pthread_mutex_lock(&chopsticks[first]),
                      "pthread_mutex_lock");

        check_pthread(pthread_mutex_lock(&chopsticks[second]),
                      "pthread_mutex_lock");

        printf("Philosopher %u is eating with chopsticks %u and %u\n",
               id,
               left,
               right);

        sleep(1);

        check_pthread(pthread_mutex_unlock(&chopsticks[second]),
                      "pthread_mutex_unlock");

        check_pthread(pthread_mutex_unlock(&chopsticks[first]),
                      "pthread_mutex_unlock");
    }

    return NULL;
}

int main(void) {
    unsigned args[THINKERS];
    pthread_t thinkers[THINKERS];

    for (size_t i = 0; i < THINKERS; i++) {
        check_pthread(pthread_mutex_init(&chopsticks[i], NULL),
                      "pthread_mutex_init");
    }

    for (size_t i = 0; i < THINKERS; i++) {
        args[i] = (unsigned)i;

        check_pthread(pthread_create(&thinkers[i], NULL, dine, &args[i]),
                      "pthread_create");
    }

    for (size_t i = 0; i < THINKERS; i++) {
        check_pthread(pthread_join(thinkers[i], NULL),
                      "pthread_join");
    }

    for (size_t i = 0; i < THINKERS; i++) {
        check_pthread(pthread_mutex_destroy(&chopsticks[i]),
                      "pthread_mutex_destroy");
    }

    return EXIT_SUCCESS;
}
