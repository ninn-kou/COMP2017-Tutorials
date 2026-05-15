// 2026 S1C COMP2017 Week 11 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// The order book has two shared linked lists: bids and asks. The writer thread appends new orders, and the matcher thread wakes every 3 seconds to find exact full matches. A single mutex protects both lists.


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct order {
    int qty;
    int price;
    int chosen;
    struct order *next;
} order;

static order *bids = NULL;       // BUY orders
static order *asks = NULL;       // SELL orders
static pthread_mutex_t book_lock = PTHREAD_MUTEX_INITIALIZER;
static int input_done = 0;

static void append(order **head, int qty, int price) {
    order *node = malloc(sizeof *node);
    if (node == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    node->qty = qty;
    node->price = price;
    node->chosen = 0;
    node->next = NULL;

    if (*head == NULL) {
        *head = node;
        return;
    }

    order *p = *head;
    while (p->next != NULL) {
        p = p->next;
    }
    p->next = node;
}

static void clear_chosen(order *head) {
    for (order *p = head; p != NULL; p = p->next) {
        p->chosen = 0;
    }
}

static int price_matches(order *o, int price, int choosing_asks) {
    if (choosing_asks) {
        return o->price <= price;  // BUY can use SELL orders at or below its price
    }

    return o->price >= price;      // SELL can use BUY orders at or above its price
}

static int choose_subset(order *head, int target_qty, int price, int choosing_asks) {
    if (target_qty == 0) {
        return 1;
    }

    for (order *p = head; p != NULL; p = p->next) {
        if (p->qty <= target_qty && price_matches(p, price, choosing_asks)) {
            p->chosen = 1;

            if (choose_subset(p->next, target_qty - p->qty, price, choosing_asks)) {
                return 1;
            }

            p->chosen = 0;
        }
    }

    return 0;
}

static void remove_chosen(order **head) {
    order **cur = head;

    while (*cur != NULL) {
        order *node = *cur;

        if (node->chosen) {
            *cur = node->next;
            free(node);
        } else {
            cur = &node->next;
        }
    }
}

static void free_orders(order *head) {
    while (head != NULL) {
        order *next = head->next;
        free(head);
        head = next;
    }
}

static void match_book(void) {
    order **bidp = &bids;

    while (*bidp != NULL) {
        order *bid = *bidp;
        clear_chosen(asks);

        if (choose_subset(asks, bid->qty, bid->price, 1)) {
            printf("TRADE %d @ %d\n", bid->qty, bid->price);
            fflush(stdout);

            remove_chosen(&asks);
            *bidp = bid->next;
            free(bid);
        } else {
            bidp = &bid->next;
        }
    }

    order **askp = &asks;

    while (*askp != NULL) {
        order *ask = *askp;
        clear_chosen(bids);

        if (choose_subset(bids, ask->qty, ask->price, 0)) {
            printf("TRADE %d @ %d\n", ask->qty, ask->price);
            fflush(stdout);

            remove_chosen(&bids);
            *askp = ask->next;
            free(ask);
        } else {
            askp = &ask->next;
        }
    }
}

static void *writer_thread(void *arg) {
    (void)arg;
    char line[64];

    while (fgets(line, sizeof line, stdin) != NULL) {
        int qty, price;

        if (sscanf(line, "BUY %d %d", &qty, &price) == 2) {
            if (qty <= 0 || price < 0) {
                fprintf(stderr, "invalid BUY order\n");
                continue;
            }

            pthread_mutex_lock(&book_lock);
            append(&bids, qty, price);
            pthread_mutex_unlock(&book_lock);
        } else if (sscanf(line, "SELL %d %d", &qty, &price) == 2) {
            if (qty <= 0 || price < 0) {
                fprintf(stderr, "invalid SELL order\n");
                continue;
            }

            pthread_mutex_lock(&book_lock);
            append(&asks, qty, price);
            pthread_mutex_unlock(&book_lock);
        } else {
            fprintf(stderr, "unknown command\n");
        }
    }

    pthread_mutex_lock(&book_lock);
    input_done = 1;
    pthread_mutex_unlock(&book_lock);

    return NULL;
}

static void *reader_thread(void *arg) {
    (void)arg;

    while (1) {
        sleep(3);

        pthread_mutex_lock(&book_lock);
        match_book();

        if (input_done) {
            pthread_mutex_unlock(&book_lock);
            break;
        }

        pthread_mutex_unlock(&book_lock);
    }

    return NULL;
}

int main(void) {
    pthread_t w, r;

    if (pthread_create(&w, NULL, writer_thread, NULL) != 0) {
        fprintf(stderr, "pthread_create writer failed\n");
        return EXIT_FAILURE;
    }

    if (pthread_create(&r, NULL, reader_thread, NULL) != 0) {
        fprintf(stderr, "pthread_create reader failed\n");
        return EXIT_FAILURE;
    }

    pthread_join(w, NULL);
    pthread_join(r, NULL);

    free_orders(bids);
    free_orders(asks);
    pthread_mutex_destroy(&book_lock);

    return EXIT_SUCCESS;
}
