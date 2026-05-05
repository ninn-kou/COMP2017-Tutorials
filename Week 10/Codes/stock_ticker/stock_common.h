// 2026 S1C COMP2017 Week 10 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Stock Ticker: stock_common.h, publisher.c, subscriber.c


#ifndef STOCK_COMMON_H
#define STOCK_COMMON_H

#define MAX_STOCKS 4

#define SHM_NAME "/comp2017_stock_ticker"
#define SEM_MUTEX_NAME "/comp2017_stock_mutex"
#define SEM_UPDATE_NAME "/comp2017_stock_update"

typedef struct {
    char name[10];
    float price;
} Stock;

typedef struct {
    Stock stocks[MAX_STOCKS];
    int updated;
} SharedData;

#endif
