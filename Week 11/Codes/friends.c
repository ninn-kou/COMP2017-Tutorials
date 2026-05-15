// 2026 S1C COMP2017 Week 11 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// https://edstem.org/au/courses/31567/lessons/99441/slides/682749


#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

int grapes = 20;
pthread_mutex_t plate = PTHREAD_MUTEX_INITIALIZER;

void eat_grape(void) {
    grapes--;
    sleep(1);
}

void *joey(void *arg) {
    (void)arg;

    pthread_mutex_lock(&plate);

    while (grapes > 0) {
        printf("Joey eats one. Grapes left: %d\n", grapes);
        eat_grape();
    }

    pthread_mutex_unlock(&plate);

    printf("Joey: JOEY DOESN'T SHARE FOOD!\n");

    return NULL;
}

void *emma(void *arg) {
    (void)arg;

    pthread_mutex_lock(&plate);

    if (grapes > 0) {
        printf("Emma eats. Grapes left: %d\n", grapes);
        eat_grape();
    }

    pthread_mutex_unlock(&plate);

    return NULL;
}

int main(void) {
    pthread_t joey_thread;
    pthread_t emma_thread;

    pthread_create(&joey_thread, NULL, joey, NULL);
    pthread_create(&emma_thread, NULL, emma, NULL);

    pthread_join(joey_thread, NULL);
    pthread_join(emma_thread, NULL);

    pthread_mutex_destroy(&plate);

    printf("Grapes left: %d\n", grapes);

    return 0;
}
