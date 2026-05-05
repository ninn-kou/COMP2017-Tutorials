// 2026 S1C COMP2017 Week 10 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Stock Ticker: stock_common.h, publisher.c, subscriber.c


#define _POSIX_C_SOURCE 200809L

#include "stock_common.h"

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

static void die(const char *msg) {
    perror(msg);
    exit(1);
}

static void wait_sem(sem_t *sem) {
    while (sem_wait(sem) == -1) {
        if (errno != EINTR) {
            die("sem_wait");
        }
    }
}

int main(void) {
    int fd = shm_open(SHM_NAME, O_RDWR, 0);

    if (fd == -1) {
        die("shm_open");
    }

    SharedData *data = mmap(NULL,
                            sizeof(SharedData),
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED,
                            fd,
                            0);

    if (data == MAP_FAILED) {
        die("mmap");
    }

    close(fd);

    sem_t *mutex = sem_open(SEM_MUTEX_NAME, 0);
    if (mutex == SEM_FAILED) {
        die("sem_open mutex");
    }

    sem_t *update = sem_open(SEM_UPDATE_NAME, 0);
    if (update == SEM_FAILED) {
        die("sem_open update");
    }

    int last_seen = 0;

    while (1) {
        /*
         * Sleep until the publisher posts an update.
         * This avoids busy waiting.
         */
        wait_sem(update);

        wait_sem(mutex);

        if (data->updated != last_seen) {
            last_seen = data->updated;

            printf("\nsubscriber: update %d\n", last_seen);

            for (int i = 0; i < MAX_STOCKS; i++) {
                printf("  %-5s %.2f\n",
                       data->stocks[i].name,
                       data->stocks[i].price);
            }

            fflush(stdout);
        }

        if (sem_post(mutex) == -1) {
            die("sem_post mutex");
        }
    }

    return 0;
}
