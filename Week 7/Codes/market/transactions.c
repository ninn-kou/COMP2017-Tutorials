// 2026 S1C COMP2017 Week 7 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Market: main.c, purchase_queue.c, purchase_queue.h, transactions.c, transactions.h


#include "transactions.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *market_strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (copy != NULL) {
        memcpy(copy, s, len);
    }
    return copy;
}

transaction *transaction_create(char *name, char *phone, char *time) {
    transaction *t = malloc(sizeof *t);
    if (t == NULL) {
        return NULL;
    }

    t->customer_name = name;
    t->phone_number = phone;
    t->checkout_time = time;
    pq_init(&t->items);
    t->next = NULL;
    return t;
}

void append_transaction(transaction **head, transaction **tail, transaction *t) {
    if (*tail == NULL) {
        *head = t;
        *tail = t;
    } else {
        (*tail)->next = t;
        *tail = t;
    }
}

static char safe_char(char c) {
    return isalnum((unsigned char)c) ? c : '_';
}

static char *make_filename(const transaction *t) {
    size_t len = strlen(t->customer_name) + strlen(t->checkout_time) + 6;
    char *filename = malloc(len);
    if (filename == NULL) {
        return NULL;
    }

    size_t pos = 0;
    for (const char *p = t->customer_name; *p != '\0'; p++) {
        filename[pos++] = safe_char(*p);
    }

    filename[pos++] = '_';

    for (const char *p = t->checkout_time; *p != '\0'; p++) {
        filename[pos++] = safe_char(*p);
    }

    strcpy(filename + pos, ".txt");
    return filename;
}

int write_receipt(transaction *t) {
    char *filename = make_filename(t);
    if (filename == NULL) {
        return 0;
    }

    FILE *fp = fopen(filename, "w");
    free(filename);

    if (fp == NULL) {
        return 0;
    }

    fprintf(fp, "Customer: %s\n", t->customer_name);
    fprintf(fp, "Phone: %s\n", t->phone_number);
    fprintf(fp, "Checkout: %s\n\n", t->checkout_time);
    fprintf(fp, "Items (sorted by price):\n");

    double total = 0.0;
    purchase *p;

    while ((p = pq_pop(&t->items)) != NULL) {
        double subtotal = p->quantity * p->cost;
        total += subtotal;

        fprintf(fp, "%s x%d @ %.2f = %.2f\n",
                p->item_name,
                p->quantity,
                p->cost,
                subtotal);

        purchase_destroy(p);
    }

    fprintf(fp, "\nTotal: %.2f\n", total);
    fclose(fp);
    return 1;
}

void transaction_destroy_list(transaction *head) {
    while (head != NULL) {
        transaction *next = head->next;
        pq_destroy(&head->items);
        free(head->customer_name);
        free(head->phone_number);
        free(head->checkout_time);
        free(head);
        head = next;
    }
}
