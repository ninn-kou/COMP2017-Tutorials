// 2026 S1C COMP2017 Week 12 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
//
// We have previously solved the dining philosophers problem by using a locking hierarchy.
// This time, use a semaphore for the table that only allows N/2 philosophers to eat at a time.
// Both semaphores and locks represent an abstract resource.
// Dining with Mutexes - Chopsticks are Shared Resource
// Dining with Semaphores - The act of dining is the Shared Resource. We are protecting the act of dining.


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define THINKERS 5

static sem_t dining_counter;
static pthread_mutex_t chopsticks[THINKERS];

void *dine(void *arg) {
    const unsigned id = *((unsigned *)arg);

    unsigned left = id;
    unsigned right = (id + 1) % THINKERS;

    // Now we have infinite number of foods so the philosopher threads never finish.
    // To see fixed number of meals, replace the following "while (true)" by
    // "for (int meal = 0; meal < 3; meal++)".
    while (true) {
        sem_wait(&dining_counter);

        pthread_mutex_lock(&chopsticks[left]);
        pthread_mutex_lock(&chopsticks[right]);

        printf("Philosopher %u is eating\n", id);
        sleep(1);

        pthread_mutex_unlock(&chopsticks[right]);
        pthread_mutex_unlock(&chopsticks[left]);

        sem_post(&dining_counter);

        /*
         * Optional thinking time.
         * This makes the output easier to observe.
         */
        sleep(1);
    }

    return NULL;
}

int main(void) {
    unsigned args[THINKERS];
    pthread_t thinkers[THINKERS];

    if (sem_init(&dining_counter, 0, THINKERS / 2) == -1) {
        perror("unable to initialize semaphore");
        return 1;
    }

    for (size_t i = 0; i < THINKERS; i++) {
        if (pthread_mutex_init(chopsticks + i, NULL) != 0) {
            perror("unable to initialize mutex");
            return 1;
        }
    }

    for (size_t i = 0; i < THINKERS; i++) {
        args[i] = (unsigned)i;

        if (pthread_create(thinkers + i, NULL, dine, args + i) != 0) {
            perror("unable to create thread");
            return 1;
        }
    }

    for (size_t i = 0; i < THINKERS; i++) {
        if (pthread_join(thinkers[i], NULL) != 0) {
            perror("unable to join thread");
            return 1;
        }
    }

    for (size_t i = 0; i < THINKERS; i++) {
        if (pthread_mutex_destroy(chopsticks + i) != 0) {
            perror("unable to destroy mutex");
            return 1;
        }
    }

    sem_destroy(&dining_counter);

    return 0;
}
