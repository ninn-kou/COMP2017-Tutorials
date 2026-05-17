// 2026 S1C COMP2017 Week 11 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Read-Write Lock Demonstration.


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int shared_value = 0;
static pthread_rwlock_t lock = PTHREAD_RWLOCK_INITIALIZER;

static void *reader(void *arg) {
    int id = *(int *)arg;

    pthread_rwlock_rdlock(&lock);

    printf("reader %d saw shared_value = %d\n", id, shared_value);

    pthread_rwlock_unlock(&lock);

    return NULL;
}

static void *writer(void *arg) {
    (void)arg;

    pthread_rwlock_wrlock(&lock);

    shared_value++;
    printf("writer updated shared_value to %d\n", shared_value);

    pthread_rwlock_unlock(&lock);

    return NULL;
}

int main(void) {
    pthread_t readers[3];
    pthread_t writer_thread;
    int ids[3] = {0, 1, 2};

    for (int i = 0; i < 3; i++) {
        int err = pthread_create(&readers[i], NULL, reader, &ids[i]);

        if (err != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(err));
            return EXIT_FAILURE;
        }
    }

    int err = pthread_create(&writer_thread, NULL, writer, NULL);

    if (err != 0) {
        fprintf(stderr, "pthread_create: %s\n", strerror(err));
        return EXIT_FAILURE;
    }

    for (int i = 0; i < 3; i++) {
        pthread_join(readers[i], NULL);
    }

    pthread_join(writer_thread, NULL);

    pthread_rwlock_destroy(&lock);
    return EXIT_SUCCESS;
}
