#define _POSIX_C_SOURCE 200809L

#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    WORKERS = 4,
    BRIGHT_THRESHOLD = 200
};

struct image_stats {
    uint64_t sum;
    uint64_t bright_count;
    uint8_t maximum;
};

struct thread_task {
    const uint8_t *pixels;
    size_t begin;
    size_t end;
    uint8_t threshold;
    struct image_stats result;
};

static struct image_stats scan_range(const uint8_t *pixels, size_t begin,
                                     size_t end, uint8_t threshold) {
    struct image_stats stats = {0, 0, 0};

    for (size_t i = begin; i < end; i++) {
        const uint8_t value = pixels[i];
        stats.sum += value;
        if (value >= threshold) {
            stats.bright_count++;
        }
        if (value > stats.maximum) {
            stats.maximum = value;
        }
    }

    return stats;
}

static void *scan_worker(void *argument) {
    struct thread_task *task = argument;
    task->result = scan_range(task->pixels, task->begin, task->end,
                              task->threshold);
    return NULL;
}

static void combine(struct image_stats *total,
                    const struct image_stats *partial) {
    total->sum += partial->sum;
    total->bright_count += partial->bright_count;
    if (partial->maximum > total->maximum) {
        total->maximum = partial->maximum;
    }
}

int main(void) {
    static const uint8_t pixels[] = {
        12, 240, 88, 201, 199, 255, 0, 32, 220,
        17, 198, 202, 64, 180, 230, 3, 99
    };
    const size_t pixel_count = sizeof pixels / sizeof pixels[0];

    pthread_t threads[WORKERS];
    struct thread_task tasks[WORKERS];
    size_t created = 0;
    int failed = 0;

    for (size_t i = 0; i < WORKERS; i++) {
        tasks[i] = (struct thread_task) {
            .pixels = pixels,
            .begin = pixel_count * i / WORKERS,
            .end = pixel_count * (i + 1) / WORKERS,
            .threshold = BRIGHT_THRESHOLD,
            .result = {0, 0, 0}
        };

        const int error = pthread_create(&threads[i], NULL, scan_worker,
                                         &tasks[i]);
        if (error != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(error));
            failed = 1;
            break;
        }
        created++;
    }

    for (size_t i = 0; i < created; i++) {
        const int error = pthread_join(threads[i], NULL);
        if (error != 0) {
            fprintf(stderr, "pthread_join: %s\n", strerror(error));
            failed = 1;
        }
    }

    if (failed || created != WORKERS) {
        fprintf(stderr, "incomplete worker set; result rejected\n");
        return EXIT_FAILURE;
    }

    struct image_stats total = {0, 0, 0};
    for (size_t i = 0; i < WORKERS; i++) {
        combine(&total, &tasks[i].result);
    }

    printf("sum=%" PRIu64 " bright=%" PRIu64 " max=%u\n",
           total.sum, total.bright_count, (unsigned) total.maximum);
    return EXIT_SUCCESS;
}
