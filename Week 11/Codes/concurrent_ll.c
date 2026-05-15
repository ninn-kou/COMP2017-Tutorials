// 2026 S1C COMP2017 Week 11 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// A concurrent linked list in C.


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>

struct node {
    int data;
    struct node *next;
};

struct ll {
    struct node *head;
    pthread_mutex_t lock;
    int active_iterators;
};

struct ll_iterator {
    struct ll *owner;
    struct node *curr;
    bool begun;
    bool active;
};

struct ll *ll_init(void);
void ll_destroy(struct ll *list);

void ll_append(struct ll *list, int value);
void ll_insert(struct ll *list, int index, int value);
void ll_remove(struct ll *list, int index);

struct ll_iterator *ll_iterator_init(struct ll *list);
void ll_iterator_destroy(struct ll_iterator *iterator);
bool ll_iterator_has_next(struct ll_iterator *iterator);
int ll_iterator_next(struct ll_iterator *iterator);
int ll_iterator_remove(struct ll_iterator *iterator);

static void die_pthread(int err, const char *message) {
    if (err != 0) {
        fprintf(stderr, "%s: %s\n", message, strerror(err));
        exit(1);
    }
}

static void deactivate_iterator_locked(struct ll_iterator *iterator) {
    if (iterator->active) {
        iterator->owner->active_iterators--;
        iterator->active = false;
    }
}

static void update_iterator_activity_locked(struct ll_iterator *iterator) {
    bool should_be_active = iterator->begun && iterator->curr != NULL;

    if (should_be_active && !iterator->active) {
        iterator->owner->active_iterators++;
        iterator->active = true;
    } else if (!should_be_active && iterator->active) {
        iterator->owner->active_iterators--;
        iterator->active = false;
    }
}

static int other_active_iterators_locked(struct ll_iterator *iterator) {
    int active = iterator->owner->active_iterators;

    if (iterator->active) {
        active--;
    }

    return active;
}

static void fail_iterator_remove_locked(struct ll_iterator *iterator) {
    struct ll *list = iterator->owner;

    /*
     * If this iterator was active, remove it from the active count
     * before terminating the calling thread.
     */
    deactivate_iterator_locked(iterator);

    pthread_mutex_unlock(&list->lock);

    fprintf(stderr,
            "ConcurrentModificationException: another iterator is still active\n");

    pthread_exit(NULL);
}

static void fail_list_write_locked(struct ll *list) {
    pthread_mutex_unlock(&list->lock);

    fprintf(stderr,
            "ConcurrentModificationException: list modified while iterator is active\n");

    pthread_exit(NULL);
}

struct ll *ll_init(void) {
    struct ll *list = malloc(sizeof *list);

    if (list == NULL) {
        perror("malloc");
        exit(1);
    }

    list->head = NULL;
    list->active_iterators = 0;

    int err = pthread_mutex_init(&list->lock, NULL);
    die_pthread(err, "pthread_mutex_init");

    return list;
}

void ll_destroy(struct ll *list) {
    if (list == NULL) {
        return;
    }

    pthread_mutex_lock(&list->lock);

    struct node *curr = list->head;

    while (curr != NULL) {
        struct node *next = curr->next;
        free(curr);
        curr = next;
    }

    list->head = NULL;

    pthread_mutex_unlock(&list->lock);
    pthread_mutex_destroy(&list->lock);
    free(list);
}

void ll_append(struct ll *list, int value) {
    struct node *new_node = malloc(sizeof *new_node);

    if (new_node == NULL) {
        perror("malloc");
        pthread_exit(NULL);
    }

    new_node->data = value;
    new_node->next = NULL;

    pthread_mutex_lock(&list->lock);

    /*
     * Extension: direct list modification is rejected while an iterator
     * is active.
     */
    if (list->active_iterators > 0) {
        pthread_mutex_unlock(&list->lock);
        free(new_node);

        fprintf(stderr,
                "ConcurrentModificationException: ll_append while iterator is active\n");

        pthread_exit(NULL);
    }

    if (list->head == NULL) {
        list->head = new_node;
    } else {
        struct node *temp = list->head;

        while (temp->next != NULL) {
            temp = temp->next;
        }

        temp->next = new_node;
    }

    pthread_mutex_unlock(&list->lock);
}

void ll_insert(struct ll *list, int index, int value) {
    struct node *new_node = malloc(sizeof *new_node);

    if (new_node == NULL) {
        perror("malloc");
        pthread_exit(NULL);
    }

    new_node->data = value;

    pthread_mutex_lock(&list->lock);

    /*
     * Extension: direct list modification is rejected while an iterator
     * is active.
     */
    if (list->active_iterators > 0) {
        pthread_mutex_unlock(&list->lock);
        free(new_node);

        fprintf(stderr,
                "ConcurrentModificationException: ll_insert while iterator is active\n");

        pthread_exit(NULL);
    }

    if (index == 0) {
        new_node->next = list->head;
        list->head = new_node;
        pthread_mutex_unlock(&list->lock);
        return;
    }

    struct node *temp = list->head;

    for (int i = 0; i < index - 1; i++) {
        if (temp == NULL) {
            pthread_mutex_unlock(&list->lock);
            free(new_node);
            assert(false);
        }

        temp = temp->next;
    }

    if (temp == NULL) {
        pthread_mutex_unlock(&list->lock);
        free(new_node);
        assert(false);
    }

    new_node->next = temp->next;
    temp->next = new_node;

    pthread_mutex_unlock(&list->lock);
}

void ll_remove(struct ll *list, int index) {
    pthread_mutex_lock(&list->lock);

    /*
     * Extension: direct list modification is rejected while an iterator
     * is active.
     */
    if (list->active_iterators > 0) {
        fail_list_write_locked(list);
    }

    if (list->head == NULL) {
        pthread_mutex_unlock(&list->lock);
        return;
    }

    struct node *temp = list->head;

    if (index == 0) {
        list->head = temp->next;
        free(temp);
        pthread_mutex_unlock(&list->lock);
        return;
    }

    struct node *prev = NULL;

    for (int i = 0; i < index; i++) {
        if (temp == NULL) {
            pthread_mutex_unlock(&list->lock);
            assert(false);
        }

        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) {
        pthread_mutex_unlock(&list->lock);
        assert(false);
    }

    prev->next = temp->next;
    free(temp);

    pthread_mutex_unlock(&list->lock);
}

struct ll_iterator *ll_iterator_init(struct ll *list) {
    struct ll_iterator *iterator = malloc(sizeof *iterator);

    if (iterator == NULL) {
        perror("malloc");
        pthread_exit(NULL);
    }

    pthread_mutex_lock(&list->lock);

    iterator->owner = list;
    iterator->curr = list->head;
    iterator->begun = false;
    iterator->active = false;

    pthread_mutex_unlock(&list->lock);

    return iterator;
}

void ll_iterator_destroy(struct ll_iterator *iterator) {
    if (iterator == NULL) {
        return;
    }

    pthread_mutex_lock(&iterator->owner->lock);
    deactivate_iterator_locked(iterator);
    pthread_mutex_unlock(&iterator->owner->lock);

    free(iterator);
}

bool ll_iterator_has_next(struct ll_iterator *iterator) {
    pthread_mutex_lock(&iterator->owner->lock);

    bool result = iterator->curr != NULL;

    if (!result) {
        update_iterator_activity_locked(iterator);
    }

    pthread_mutex_unlock(&iterator->owner->lock);

    return result;
}

int ll_iterator_next(struct ll_iterator *iterator) {
    pthread_mutex_lock(&iterator->owner->lock);

    if (iterator->curr == NULL) {
        pthread_mutex_unlock(&iterator->owner->lock);
        fprintf(stderr, "ll_iterator_next called with no next element\n");
        pthread_exit(NULL);
    }

    int value = iterator->curr->data;

    iterator->curr = iterator->curr->next;
    iterator->begun = true;

    update_iterator_activity_locked(iterator);

    pthread_mutex_unlock(&iterator->owner->lock);

    return value;
}

int ll_iterator_remove(struct ll_iterator *iterator) {
    pthread_mutex_lock(&iterator->owner->lock);

    if (iterator->curr == NULL) {
        pthread_mutex_unlock(&iterator->owner->lock);
        fprintf(stderr, "ll_iterator_remove called with no next element\n");
        pthread_exit(NULL);
    }

    /*
     * The key check:
     * removing through this iterator is allowed only if no other iterator
     * is currently active.
     */
    if (other_active_iterators_locked(iterator) > 0) {
        fail_iterator_remove_locked(iterator);
    }

    struct node *temp = iterator->owner->head;
    struct node *prev = NULL;

    while (temp != NULL && temp != iterator->curr) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) {
        pthread_mutex_unlock(&iterator->owner->lock);
        fprintf(stderr, "iterator points to a node that is no longer in the list\n");
        pthread_exit(NULL);
    }

    struct node *next_node = temp->next;
    int value = temp->data;

    if (prev == NULL) {
        iterator->owner->head = temp->next;
    } else {
        prev->next = temp->next;
    }

    free(temp);

    iterator->curr = next_node;
    iterator->begun = true;

    update_iterator_activity_locked(iterator);

    pthread_mutex_unlock(&iterator->owner->lock);

    return value;
}

/* sample test code */
static struct ll *list;

void *thread_A_worker(void *arg) {
    (void)arg;

    struct ll_iterator *iterator = ll_iterator_init(list);
    int idx = 0;

    while (ll_iterator_has_next(iterator)) {
        if (idx != 1) {
            int value = ll_iterator_next(iterator);
            printf("Thread A: %d\n", value);
        } else {
            /*
             * Removing from the same iterator is okay.
             */
            int value = ll_iterator_remove(iterator);
            printf("Thread A remove: %d\n", value);
        }

        idx++;
        sleep(1);
    }

    ll_iterator_destroy(iterator);
    return NULL;
}

void *thread_B_worker(void *arg) {
    (void)arg;

    sleep(2);

    struct ll_iterator *iterator = ll_iterator_init(list);

    /*
     * This should detect concurrent modification and terminate this thread.
     */
    ll_iterator_remove(iterator);

    /*
     * We should not reach this point.
     */
    assert(false);

    ll_iterator_destroy(iterator);
    return NULL;
}

int main(void) {
    list = ll_init();

    ll_append(list, 1);
    ll_append(list, 2);
    ll_append(list, 3);
    ll_append(list, 4);

    pthread_t threadA;
    pthread_t threadB;

    int err = pthread_create(&threadA, NULL, thread_A_worker, NULL);
    die_pthread(err, "pthread_create");

    err = pthread_create(&threadB, NULL, thread_B_worker, NULL);
    die_pthread(err, "pthread_create");

    err = pthread_join(threadA, NULL);
    die_pthread(err, "pthread_join");

    err = pthread_join(threadB, NULL);
    die_pthread(err, "pthread_join");

    ll_destroy(list);

    return 0;
}
