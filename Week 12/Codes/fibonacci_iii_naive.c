// 2026 S1C COMP2017 Week 12 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Fibonacci Numbers - III
// By creating more threads managed by semaphores without max threads limitations.
// Do NOT run it with large n like 42. It creates far too many threads.


#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int n;
    long result;
    sem_t done;
} FibTask;

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

long fib(int n);

static void *fib_worker(void *arg) {
    FibTask *task = arg;

    task->result = fib(task->n);

    if (sem_post(&task->done) == -1) {
        perror("sem_post");
        exit(EXIT_FAILURE);
    }

    return NULL;
}

long fib(int n) {
    if (n <= 1) {
        return n;
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

int main(void) {
    printf("%ld\n", fib(10));
    return 0;
}
