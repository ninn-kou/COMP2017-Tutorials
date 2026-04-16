// 2026 S1C COMP2017 Week 7 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Market: main.c, purchase_queue.c, purchase_queue.h, transactions.c, transactions.h


#ifndef PURCHASE_QUEUE_H
#define PURCHASE_QUEUE_H

#include <stddef.h>

typedef struct purchase {
    char *item_name;
    int quantity;
    double cost;
} purchase;

typedef struct purchase_queue {
    purchase **data;
    size_t size;
    size_t capacity;
} purchase_queue;

purchase *purchase_create(const char *name, int quantity, double cost);
void purchase_destroy(purchase *p);

void pq_init(purchase_queue *pq);
int pq_push(purchase_queue *pq, purchase *p);
purchase *pq_pop(purchase_queue *pq);
void pq_destroy(purchase_queue *pq);

#endif
