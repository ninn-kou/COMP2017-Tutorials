// 2026 S1C COMP2017 Week 10 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Kitchen of Hell: https://edstem.org/au/courses/31567/lessons/99441/slides/682746


#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NUM_MEALS 4

static atomic_int meals_ready = 0;

static int req_wine = 0;
static int req_stock = 0;
static int req_herbs = 0;

typedef struct {
    const char *component;
    void (*prep)(void);
    int *count;
} SousChefTask;

static void check_pthread(int err, const char *message) {
    if (err != 0) {
        fprintf(stderr, "%s: %s\n", message, strerror(err));
        exit(EXIT_FAILURE);
    }
}

static void prep_wine(void) {
    sleep(1);
}

static void prep_stock(void) {
    sleep(2);
}

static void prep_herbs(void) {
    sleep(1);
}

static void *sous_chef(void *arg) {
    SousChefTask *task = arg;

    for (int i = 0; i < NUM_MEALS; i++) {
        task->prep();

        (*task->count)++;

        printf("%s Chef: %s Ready!\n",
               task->component,
               task->component);
        fflush(stdout);
    }

    return NULL;
}

static void *head_chef(void *arg) {
    (void)arg;

    pthread_t sous_chefs[3];

    SousChefTask tasks[3] = {
        {"Red Wine", prep_wine, &req_wine},
        {"Lamb Stock", prep_stock, &req_stock},
        {"Herb Seasoning", prep_herbs, &req_herbs}
    };

    for (int i = 0; i < 3; i++) {
        int err = pthread_create(&sous_chefs[i], NULL, sous_chef, &tasks[i]);
        check_pthread(err, "pthread_create");
    }

    for (int i = 0; i < 3; i++) {
        int err = pthread_join(sous_chefs[i], NULL);
        check_pthread(err, "pthread_join");
    }

    if (req_wine == NUM_MEALS &&
        req_stock == NUM_MEALS &&
        req_herbs == NUM_MEALS) {
        printf("Head Chef: All lamb sauce ready!\n");
        fflush(stdout);

        atomic_store(&meals_ready, NUM_MEALS);
    }

    return NULL;
}

static void *rordon(void *arg) {
    (void)arg;

    pthread_t head;

    int err = pthread_create(&head, NULL, head_chef, NULL);
    check_pthread(err, "pthread_create");

    while (atomic_load(&meals_ready) < NUM_MEALS) {
        printf("Rordon: WHERE IS THE LAMB SAUCE\n");
        fflush(stdout);
        sleep(1);
    }

    err = pthread_join(head, NULL);
    check_pthread(err, "pthread_join");

    return NULL;
}

int main(void) {
    pthread_t rordon_thread;

    int err = pthread_create(&rordon_thread, NULL, rordon, NULL);
    check_pthread(err, "pthread_create");

    err = pthread_join(rordon_thread, NULL);
    check_pthread(err, "pthread_join");

    return EXIT_SUCCESS;
}
