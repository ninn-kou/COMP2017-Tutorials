// 2026 S1C COMP2017 Week 7 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Market: main.c, purchase_queue.c, purchase_queue.h, transactions.c, transactions.h


#include "purchase_queue.h"

#include <stdlib.h>
#include <string.h>

static char *pq_strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (copy != NULL) {
        memcpy(copy, s, len);
    }
    return copy;
}

purchase *purchase_create(const char *name, int quantity, double cost) {
    purchase *p = malloc(sizeof *p);
    if (p == NULL) {
        return NULL;
    }

    p->item_name = pq_strdup(name);
    if (p->item_name == NULL) {
        free(p);
        return NULL;
    }

    p->quantity = quantity;
    p->cost = cost;
    return p;
}

void purchase_destroy(purchase *p) {
    if (p == NULL) {
        return;
    }

    free(p->item_name);
    free(p);
}

void pq_init(purchase_queue *pq) {
    pq->data = NULL;
    pq->size = 0;
    pq->capacity = 0;
}

static int purchase_less(const purchase *a, const purchase *b) {
    if (a->cost < b->cost) {
        return 1;
    }
    if (a->cost > b->cost) {
        return 0;
    }
    return strcmp(a->item_name, b->item_name) < 0;
}

static void pq_swap(purchase **a, purchase **b) {
    purchase *tmp = *a;
    *a = *b;
    *b = tmp;
}

int pq_push(purchase_queue *pq, purchase *p) {
    if (pq->size == pq->capacity) {
        size_t new_capacity = (pq->capacity == 0) ? 4 : pq->capacity * 2;
        purchase **new_data = realloc(pq->data, new_capacity * sizeof *new_data);
        if (new_data == NULL) {
            return 0;
        }
        pq->data = new_data;
        pq->capacity = new_capacity;
    }

    size_t i = pq->size;
    pq->data[pq->size] = p;
    pq->size++;

    while (i > 0) {
        size_t parent = (i - 1) / 2;
        if (!purchase_less(pq->data[i], pq->data[parent])) {
            break;
        }
        pq_swap(&pq->data[i], &pq->data[parent]);
        i = parent;
    }

    return 1;
}

static void pq_heapify_down(purchase_queue *pq, size_t i) {
    while (1) {
        size_t left = 2 * i + 1;
        size_t right = 2 * i + 2;
        size_t smallest = i;

        if (left < pq->size && purchase_less(pq->data[left], pq->data[smallest])) {
            smallest = left;
        }
        if (right < pq->size && purchase_less(pq->data[right], pq->data[smallest])) {
            smallest = right;
        }

        if (smallest == i) {
            break;
        }

        pq_swap(&pq->data[i], &pq->data[smallest]);
        i = smallest;
    }
}

purchase *pq_pop(purchase_queue *pq) {
    if (pq->size == 0) {
        return NULL;
    }

    purchase *top = pq->data[0];
    pq->size--;

    if (pq->size > 0) {
        pq->data[0] = pq->data[pq->size];
        pq_heapify_down(pq, 0);
    }

    return top;
}

void pq_destroy(purchase_queue *pq) {
    for (size_t i = 0; i < pq->size; i++) {
        purchase_destroy(pq->data[i]);
    }
    free(pq->data);
}
