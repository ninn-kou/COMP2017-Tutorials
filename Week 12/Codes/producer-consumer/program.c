// 2026 S1C COMP2017 Week 12 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// A Producer-Consumer queue is a design used to decouple tasks.
// A "Producer" thread generates data and places it into a shared buffer, while one or more "Consumer" threads wait for items to appear in that queue to process them.
// program.c uses the producer-consumer design and has 1 producer thread, which reads database queries from stdin, and 3 consumer threads which execute them using a mock database.
// In this task, you will need to initialize the semaphore and mutex, use the semaphore to keep track of items in the queue, and only wake up the consumer thread when there is work to do.


#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>

#define NUM_WORKERS 3

typedef struct Node {
    char *query;
    struct Node *next;
} Node;

static Node *head = NULL;
static Node *tail = NULL;

static pthread_mutex_t queue_lock;
static sem_t work_available;

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

static void die_pthread(int err, const char *msg) {
    if (err != 0) {
        fprintf(stderr, "%s: %s\n", msg, strerror(err));
        exit(EXIT_FAILURE);
    }
}

static void sem_wait_checked(sem_t *sem) {
    while (sem_wait(sem) == -1) {
        if (errno != EINTR) {
            die("sem_wait");
        }
    }
}

void enqueue(char *query) {
    Node *newNode = malloc(sizeof *newNode);
    if (newNode == NULL) {
        die("malloc");
    }

    newNode->query = query ? strdup(query) : NULL;
    if (query != NULL && newNode->query == NULL) {
        free(newNode);
        die("strdup");
    }

    newNode->next = NULL;

    die_pthread(pthread_mutex_lock(&queue_lock), "pthread_mutex_lock");

    if (tail == NULL) {
        head = tail = newNode;
    } else {
        tail->next = newNode;
        tail = newNode;
    }

    die_pthread(pthread_mutex_unlock(&queue_lock), "pthread_mutex_unlock");

    if (sem_post(&work_available) == -1) {
        die("sem_post");
    }
}

char *dequeue(void) {
    sem_wait_checked(&work_available);

    die_pthread(pthread_mutex_lock(&queue_lock), "pthread_mutex_lock");

    Node *temp = head;
    if (temp == NULL) {
        die_pthread(pthread_mutex_unlock(&queue_lock), "pthread_mutex_unlock");
        fprintf(stderr, "queue error: semaphore signalled but queue was empty\n");
        exit(EXIT_FAILURE);
    }

    char *query = temp->query;

    head = head->next;
    if (head == NULL) {
        tail = NULL;
    }

    die_pthread(pthread_mutex_unlock(&queue_lock), "pthread_mutex_unlock");

    free(temp);
    return query;
}

void *consumer_routine(void *arg) {
    int id = *(int *)arg;

    while (1) {
        char *query = dequeue();

        if (query == NULL) {
            return NULL;
        }

        char command[512];
        snprintf(command, sizeof command, "./database \"%s\"", query);

        FILE *fp = popen(command, "r");
        if (fp == NULL) {
            perror("popen failed");
            free(query);
            continue;
        }

        printf("[Consumer %d] Started query: %s\n", id, query);
        fflush(stdout);

        char result_buffer[1024];

        while (fgets(result_buffer, sizeof result_buffer, fp) != NULL) {
            printf("[Consumer %d Output]: %s", id, result_buffer);
            fflush(stdout);
        }

        pclose(fp);
        free(query);
    }
}

int main(void) {
    pthread_t workers[NUM_WORKERS];
    int ids[NUM_WORKERS] = {1, 2, 3};

    die_pthread(pthread_mutex_init(&queue_lock, NULL), "pthread_mutex_init");

    if (sem_init(&work_available, 0, 0) == -1) {
        die("sem_init");
    }

    for (int i = 0; i < NUM_WORKERS; i++) {
        die_pthread(
            pthread_create(&workers[i], NULL, consumer_routine, &ids[i]),
            "pthread_create"
        );
    }

    char buffer[256];

    printf("> ");

    while (fgets(buffer, sizeof buffer, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strlen(buffer) > 0) {
            enqueue(buffer);
        }
    }

    for (int i = 0; i < NUM_WORKERS; i++) {
        enqueue(NULL);
    }

    for (int i = 0; i < NUM_WORKERS; i++) {
        die_pthread(pthread_join(workers[i], NULL), "pthread_join");
    }

    if (sem_destroy(&work_available) == -1) {
        die("sem_destroy");
    }

    die_pthread(pthread_mutex_destroy(&queue_lock), "pthread_mutex_destroy");

    return EXIT_SUCCESS;
}
