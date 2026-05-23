// 2026 S1C COMP2017 Week 12 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)


#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define DISTANCE 10

int race_won = 0;
sem_t tortoise_done;

/*
 * Print out I win or I lose from each thread.
 * Returns 1 when you are the first to finish.
 */
int reach_finish(void) {
    if (race_won == 0) {
        race_won = 1;
        return 1;
    }

    return 0;
}

static void wait_for_semaphore(sem_t *sem) {
    while (sem_wait(sem) == -1) {
        if (errno != EINTR) {
            perror("sem_wait");
            exit(EXIT_FAILURE);
        }
    }
}

void *hare(void *arg) {
    (void)arg;

    int progress = 0;

    while (progress < DISTANCE) {
        progress++;
        printf("Hare: progress %d/%d\n", progress, DISTANCE);

        /*
         * The hare is fast, so no sleep here.
         */
    }

    /*
     * The hare has reached the finish line, but is not allowed
     * to claim the win until the tortoise has finished first.
     */
    wait_for_semaphore(&tortoise_done);

    if (reach_finish()) {
        printf("Hare: I win!\n");
    } else {
        printf("Hare: I lose!\n");
    }

    return NULL;
}

void *tortoise(void *arg) {
    (void)arg;

    int progress = 0;

    while (progress < DISTANCE) {
        progress++;
        printf("Tortoise: progress %d/%d\n", progress, DISTANCE);

        /*
         * The tortoise is slower.
         */
        sleep(1);
    }

    if (reach_finish()) {
        printf("Tortoise: I win!\n");
    } else {
        printf("Tortoise: I lose!\n");
    }

    /*
     * Wake the hare only after the tortoise has claimed the finish.
     */
    sem_post(&tortoise_done);

    return NULL;
}

int main(void) {
    pthread_t hare_thread;
    pthread_t tortoise_thread;

    if (sem_init(&tortoise_done, 0, 0) == -1) {
        perror("sem_init");
        return EXIT_FAILURE;
    }

    pthread_create(&hare_thread, NULL, hare, NULL);
    pthread_create(&tortoise_thread, NULL, tortoise, NULL);

    pthread_join(hare_thread, NULL);
    pthread_join(tortoise_thread, NULL);

    sem_destroy(&tortoise_done);

    return EXIT_SUCCESS;
}
