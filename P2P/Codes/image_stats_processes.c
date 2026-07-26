#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

enum {
    WORKERS = 4,
    BRIGHT_THRESHOLD = 200
};

struct image_stats {
    uint64_t sum;
    uint64_t bright_count;
    uint8_t maximum;
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

static int write_full(int fd, const void *buffer, size_t length) {
    const unsigned char *cursor = buffer;

    while (length > 0) {
        const ssize_t written = write(fd, cursor, length);
        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (written == 0) {
            errno = EIO;
            return -1;
        }
        cursor += (size_t) written;
        length -= (size_t) written;
    }

    return 0;
}

static int read_full(int fd, void *buffer, size_t length) {
    unsigned char *cursor = buffer;

    while (length > 0) {
        const ssize_t received = read(fd, cursor, length);
        if (received < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (received == 0) {
            errno = EIO;
            return -1;
        }
        cursor += (size_t) received;
        length -= (size_t) received;
    }

    return 0;
}

static pid_t waitpid_nointr(pid_t pid, int *status) {
    pid_t result;
    do {
        result = waitpid(pid, status, 0);
    } while (result < 0 && errno == EINTR);
    return result;
}

static void combine(struct image_stats *total,
                    const struct image_stats *partial) {
    total->sum += partial->sum;
    total->bright_count += partial->bright_count;
    if (partial->maximum > total->maximum) {
        total->maximum = partial->maximum;
    }
}

static void run_child(size_t worker_index, int result_pipe[WORKERS][2],
                      const uint8_t *pixels, size_t pixel_count) {
    for (size_t i = 0; i < WORKERS; i++) {
        close(result_pipe[i][0]);
        if (i != worker_index) {
            close(result_pipe[i][1]);
        }
    }

    const size_t begin = pixel_count * worker_index / WORKERS;
    const size_t end = pixel_count * (worker_index + 1) / WORKERS;
    const struct image_stats partial =
        scan_range(pixels, begin, end, BRIGHT_THRESHOLD);

    const int write_status = write_full(result_pipe[worker_index][1],
                                        &partial, sizeof partial);
    close(result_pipe[worker_index][1]);
    _exit(write_status == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}

int main(void) {
    static const uint8_t pixels[] = {
        12, 240, 88, 201, 199, 255, 0, 32, 220,
        17, 198, 202, 64, 180, 230, 3, 99
    };
    const size_t pixel_count = sizeof pixels / sizeof pixels[0];

    int result_pipe[WORKERS][2];
    size_t pipe_count = 0;
    for (; pipe_count < WORKERS; pipe_count++) {
        if (pipe(result_pipe[pipe_count]) < 0) {
            perror("pipe");
            for (size_t i = 0; i < pipe_count; i++) {
                close(result_pipe[i][0]);
                close(result_pipe[i][1]);
            }
            return EXIT_FAILURE;
        }
    }

    pid_t children[WORKERS];
    size_t created = 0;
    bool spawn_ok = true;

    for (size_t i = 0; i < WORKERS; i++) {
        const pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            spawn_ok = false;
            break;
        }
        if (pid == 0) {
            run_child(i, result_pipe, pixels, pixel_count);
        }
        children[i] = pid;
        created++;
    }

    for (size_t i = 0; i < WORKERS; i++) {
        close(result_pipe[i][1]);
        if (i >= created) {
            close(result_pipe[i][0]);
        }
    }

    struct image_stats partial[WORKERS];
    bool data_ok = true;
    for (size_t i = 0; i < created; i++) {
        if (read_full(result_pipe[i][0], &partial[i],
                      sizeof partial[i]) < 0) {
            perror("read_full");
            data_ok = false;
        }
        close(result_pipe[i][0]);
    }

    bool children_ok = true;
    for (size_t i = 0; i < created; i++) {
        int status;
        const pid_t result = waitpid_nointr(children[i], &status);
        if (result != children[i] || !WIFEXITED(status)
            || WEXITSTATUS(status) != EXIT_SUCCESS) {
            children_ok = false;
        }
    }

    if (!spawn_ok || created != WORKERS || !data_ok || !children_ok) {
        fprintf(stderr, "incomplete worker set; result rejected\n");
        return EXIT_FAILURE;
    }

    struct image_stats total = {0, 0, 0};
    for (size_t i = 0; i < WORKERS; i++) {
        combine(&total, &partial[i]);
    }

    printf("sum=%" PRIu64 " bright=%" PRIu64 " max=%u\n",
           total.sum, total.bright_count, (unsigned) total.maximum);
    return EXIT_SUCCESS;
}
