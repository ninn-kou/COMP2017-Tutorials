// 2026 S1C COMP2017 Week 10 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Sum of very large array by threads to avoid any overflow.


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SIZE 10000

typedef struct {
    int *arr;
    int start;
    int end;
    long long partial_sum;
} ThreadTask;

int *get_nums(void) {
    srand((unsigned)time(NULL));

    int *arr = malloc(sizeof(int) * SIZE);

    if (arr == NULL) {
        return NULL;
    }

    for (int i = 0; i < SIZE; i++) {
        arr[i] = rand() % 10000;
    }

    return arr;
}

static void *sum_range(void *arg) {
    ThreadTask *task = arg;
    long long total = 0;

    for (int i = task->start; i < task->end; i++) {
        total += task->arr[i];
    }

    task->partial_sum = total;
    return NULL;
}

int main(void) {
    int nthreads;

    printf("Enter number of threads: ");

    if (scanf("%d", &nthreads) != 1 || nthreads <= 0 || nthreads > SIZE) {
        fprintf(stderr, "Invalid number of threads\n");
        return EXIT_FAILURE;
    }

    int *large_array = get_nums();

    if (large_array == NULL) {
        fprintf(stderr, "Failed to allocate array\n");
        return EXIT_FAILURE;
    }

    pthread_t *threads = malloc(sizeof(pthread_t) * nthreads);
    ThreadTask *tasks = calloc((size_t)nthreads, sizeof(ThreadTask));

    if (threads == NULL || tasks == NULL) {
        fprintf(stderr, "Failed to allocate thread data\n");
        free(threads);
        free(tasks);
        free(large_array);
        return EXIT_FAILURE;
    }

    int base = SIZE / nthreads;
    int rem = SIZE % nthreads;
    int start = 0;

    for (int i = 0; i < nthreads; i++) {
        int amount = base;

        if (i < rem) {
            amount++;
        }

        tasks[i].arr = large_array;
        tasks[i].start = start;
        tasks[i].end = start + amount;
        tasks[i].partial_sum = 0;

        start = tasks[i].end;

        int err = pthread_create(&threads[i], NULL, sum_range, &tasks[i]);

        if (err != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(err));

            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }

            free(threads);
            free(tasks);
            free(large_array);
            return EXIT_FAILURE;
        }
    }

    long long sum = 0;

    for (int i = 0; i < nthreads; i++) {
        int err = pthread_join(threads[i], NULL);

        if (err != 0) {
            fprintf(stderr, "pthread_join: %s\n", strerror(err));
            free(threads);
            free(tasks);
            free(large_array);
            return EXIT_FAILURE;
        }

        sum += tasks[i].partial_sum;
    }

    printf("sum = %lld\n", sum);

    free(threads);
    free(tasks);
    free(large_array);

    return EXIT_SUCCESS;
}
