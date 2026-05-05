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
#include <string.h>
#include <sys/mman.h>
#include <time.h>
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

static void init_stocks(SharedData *data) {
    const char *names[MAX_STOCKS] = {
        "AAPL",
        "MSFT",
        "TSLA",
        "NVDA"
    };

    float prices[MAX_STOCKS] = {
        190.0f,
        410.0f,
        180.0f,
        850.0f
    };

    for (int i = 0; i < MAX_STOCKS; i++) {
        snprintf(data->stocks[i].name, sizeof data->stocks[i].name,
                 "%s", names[i]);
        data->stocks[i].price = prices[i];
    }

    data->updated = 0;
}

int main(void) {
    /*
     * Reset old objects from a previous run.
     * Do not run two publishers at the same time with this simple version.
     */
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_MUTEX_NAME);
    sem_unlink(SEM_UPDATE_NAME);

    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);

    if (fd == -1) {
        die("shm_open");
    }

    if (ftruncate(fd, sizeof(SharedData)) == -1) {
        die("ftruncate");
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

    sem_t *mutex = sem_open(SEM_MUTEX_NAME, O_CREAT | O_EXCL, 0600, 1);
    if (mutex == SEM_FAILED) {
        die("sem_open mutex");
    }

    sem_t *update = sem_open(SEM_UPDATE_NAME, O_CREAT | O_EXCL, 0600, 0);
    if (update == SEM_FAILED) {
        die("sem_open update");
    }

    init_stocks(data);

    srand((unsigned)time(NULL));

    while (1) {
        sleep(1);

        int index = rand() % MAX_STOCKS;
        float change = ((float)(rand() % 201) - 100.0f) / 100.0f;

        wait_sem(mutex);

        data->stocks[index].price += change;

        if (data->stocks[index].price < 0.0f) {
            data->stocks[index].price = 0.0f;
        }

        data->updated++;

        int update_no = data->updated;
        Stock changed = data->stocks[index];

        if (sem_post(mutex) == -1) {
            die("sem_post mutex");
        }

        printf("publisher: update %d: %s = %.2f\n",
               update_no,
               changed.name,
               changed.price);
        fflush(stdout);

        if (sem_post(update) == -1) {
            die("sem_post update");
        }
    }

    return 0;
}
