// 2026 S1C COMP2017 Week 7 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Market: main.c, purchase_queue.c, purchase_queue.h, transactions.c, transactions.h


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "transactions.h"

static int read_line(char *buf, size_t size) {
    if (fgets(buf, size, stdin) == NULL) {
        return 0;
    }

    buf[strcspn(buf, "\n")] = '\0';
    return 1;
}

int main(void) {
    transaction *head = NULL;
    transaction *tail = NULL;
    char line[256];
    int stop = 0;

    while (!stop) {
        printf("Customer name: ");
        fflush(stdout);
        if (!read_line(line, sizeof line)) {
            break;
        }
        if (line[0] == '\0') {
            continue;
        }

        char *name = market_strdup(line);
        if (name == NULL) {
            fprintf(stderr, "Allocation failed\n");
            transaction_destroy_list(head);
            return 1;
        }

        printf("Phone number: ");
        fflush(stdout);
        if (!read_line(line, sizeof line)) {
            free(name);
            break;
        }

        char *phone = market_strdup(line);
        if (phone == NULL) {
            free(name);
            transaction_destroy_list(head);
            return 1;
        }

        printf("Checkout time: ");
        fflush(stdout);
        if (!read_line(line, sizeof line)) {
            free(name);
            free(phone);
            break;
        }

        char *time = market_strdup(line);
        if (time == NULL) {
            free(name);
            free(phone);
            transaction_destroy_list(head);
            return 1;
        }

        transaction *t = transaction_create(name, phone, time);
        if (t == NULL) {
            free(name);
            free(phone);
            free(time);
            transaction_destroy_list(head);
            return 1;
        }

        append_transaction(&head, &tail, t);

        while (1) {
            printf("Item quantity cost (or DONE): ");
            fflush(stdout);

            if (!read_line(line, sizeof line)) {
                stop = 1;
                break;
            }

            if (strcmp(line, "DONE") == 0) {
                break;
            }

            char item_name[128];
            int quantity;
            double cost;

            if (sscanf(line, "%127s %d %lf", item_name, &quantity, &cost) != 3 || quantity < 0) {
                printf("Invalid entry\n");
                continue;
            }

            purchase *p = purchase_create(item_name, quantity, cost);
            if (p == NULL || !pq_push(&t->items, p)) {
                purchase_destroy(p);
                fprintf(stderr, "Allocation failed\n");
                transaction_destroy_list(head);
                return 1;
            }
        }
    }

    for (transaction *cur = head; cur != NULL; cur = cur->next) {
        if (!write_receipt(cur)) {
            fprintf(stderr, "Could not write receipt for %s\n", cur->customer_name);
        }
    }

    transaction_destroy_list(head);
    return 0;
}
