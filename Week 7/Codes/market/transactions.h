// 2026 S1C COMP2017 Week 7 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Market: main.c, purchase_queue.c, purchase_queue.h, transactions.c, transactions.h


#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H

#include "purchase_queue.h"

typedef struct transaction {
    char *customer_name;
    char *phone_number;
    char *checkout_time;
    purchase_queue items;
    struct transaction *next;
} transaction;

char *market_strdup(const char *s);

transaction *transaction_create(char *name, char *phone, char *time);
void append_transaction(transaction **head, transaction **tail, transaction *t);
int write_receipt(transaction *t);
void transaction_destroy_list(transaction *head);

#endif
