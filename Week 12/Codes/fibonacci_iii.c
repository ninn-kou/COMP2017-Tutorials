// 2026 S1C COMP2017 Week 12 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Fibonacci Numbers - III
// By creating more threads managed by semaphores within the range of limitations, and in each thread using recursions.


#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORKER_THREADS 8

typedef struct {
    int n;
    long result;
    sem_t done;
} FibTask;

static sem_t thread_slots;

static void check_pthread(int err, const char *msg) {
    if (err != 0) {
        fprintf(stderr, "%s: %s\n", msg, strerror(err));
        exit(EXIT_FAILURE);
    }
}

static void wait_sem(sem_t *sem) {
    while (sem_wait(sem) == -1) {
        if (errno != EINTR) {
            perror("sem_wait");
            exit(EXIT_FAILURE);
        }
    }
}

static int try_take_slot(void) {
    while (sem_trywait(&thread_slots) == -1) {
        if (errno == EINTR) {
            continue;
        }

        if (errno == EAGAIN) {
            return 0;
        }

        perror("sem_trywait");
        exit(EXIT_FAILURE);
    }

    return 1;
}

long fib_seq(int n) {
    if (n <= 1) {
        return n;
    }

    return fib_seq(n - 1) + fib_seq(n - 2);
}

static long fib_bounded_rec(int n);

static void *fib_worker(void *arg) {
    FibTask *task = arg;

    task->result = fib_bounded_rec(task->n);

    if (sem_post(&task->done) == -1) {
        perror("sem_post done");
        exit(EXIT_FAILURE);
    }

    /*
     * This worker thread is finished, so return its slot.
     */
    if (sem_post(&thread_slots) == -1) {
        perror("sem_post thread_slots");
        exit(EXIT_FAILURE);
    }

    return NULL;
}

static long fib_bounded_rec(int n) {
    if (n <= 1) {
        return n;
    }

    /*
     * Try to reserve two worker-thread slots.
     * If we cannot reserve both, compute this whole split sequentially.
     */
    int got_left_slot = try_take_slot();
    int got_right_slot = 0;

    if (got_left_slot) {
        got_right_slot = try_take_slot();
    }

    if (!got_left_slot || !got_right_slot) {
        if (got_left_slot) {
            if (sem_post(&thread_slots) == -1) {
                perror("sem_post thread_slots");
                exit(EXIT_FAILURE);
            }
        }

        return fib_seq(n - 1) + fib_seq(n - 2);
    }

    FibTask left = {.n = n - 1, .result = 0};
    FibTask right = {.n = n - 2, .result = 0};

    if (sem_init(&left.done, 0, 0) == -1) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    if (sem_init(&right.done, 0, 0) == -1) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    pthread_t left_thread;
    pthread_t right_thread;

    check_pthread(pthread_create(&left_thread, NULL, fib_worker, &left),
                  "pthread_create");
    check_pthread(pthread_detach(left_thread), "pthread_detach");

    check_pthread(pthread_create(&right_thread, NULL, fib_worker, &right),
                  "pthread_create");
    check_pthread(pthread_detach(right_thread), "pthread_detach");

    wait_sem(&left.done);
    wait_sem(&right.done);

    long result = left.result + right.result;

    sem_destroy(&left.done);
    sem_destroy(&right.done);

    return result;
}

long fib(int n) {
    if (sem_init(&thread_slots, 0, MAX_WORKER_THREADS) == -1) {
        perror("sem_init thread_slots");
        exit(EXIT_FAILURE);
    }

    long result = fib_bounded_rec(n);

    if (sem_destroy(&thread_slots) == -1) {
        perror("sem_destroy thread_slots");
        exit(EXIT_FAILURE);
    }

    return result;
}

int main(void) {
    printf("%ld\n", fib(42));
    return 0;
}
