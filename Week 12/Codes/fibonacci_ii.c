// 2026 S1C COMP2017 Week 12 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Fibonacci Numbers - II
// By creating two threads on the top level, and in each thread using recursions.


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int n;
    long result;
} fib_task_t;

static void check_pthread(int err, const char *message) {
    if (err != 0) {
        fprintf(stderr, "%s: %s\n", message, strerror(err));
        exit(EXIT_FAILURE);
    }
}

long fib_seq(int n) {
    if (n <= 1) {
        return n;
    }

    return fib_seq(n - 1) + fib_seq(n - 2);
}

static void *fib_worker(void *arg) {
    fib_task_t *task = arg;

    task->result = fib_seq(task->n);

    return NULL;
}

static void *fib_combiner(void *arg) {
    fib_task_t *task = arg;

    if (task->n <= 1) {
        task->result = task->n;
        return NULL;
    }

    pthread_t t2;
    pthread_t t3;

    fib_task_t left = {
        .n = task->n - 1,
        .result = 0
    };

    fib_task_t right = {
        .n = task->n - 2,
        .result = 0
    };

    check_pthread(pthread_create(&t2, NULL, fib_worker, &left),
                  "pthread_create");

    check_pthread(pthread_create(&t3, NULL, fib_worker, &right),
                  "pthread_create");

    check_pthread(pthread_join(t2, NULL), "pthread_join");
    check_pthread(pthread_join(t3, NULL), "pthread_join");

    task->result = left.result + right.result;

    return NULL;
}

long fib(int n) {
    pthread_t t1;

    fib_task_t task = {
        .n = n,
        .result = 0
    };

    check_pthread(pthread_create(&t1, NULL, fib_combiner, &task),
                  "pthread_create");

    check_pthread(pthread_join(t1, NULL), "pthread_join");

    return task.result;
}

int main(void) {
    printf("%ld\n", fib(42));
    return 0;
}
