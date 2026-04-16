// 2026 S1C COMP2017 Week 7 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Tiny Delegator: main.c, ops.c, ops.h, program.c, program.h


#include "ops.h"

#include <string.h>

int add(int a, int b) {
    return a + b;
}

int sub(int a, int b) {
    return a - b;
}

int mul(int a, int b) {
    return a * b;
}

int divide_int(int a, int b) {
    return a / b;
}

binop_fn parse_operation(const char *name) {
    if (strcmp(name, "ADD") == 0) {
        return add;
    }
    if (strcmp(name, "SUB") == 0) {
        return sub;
    }
    if (strcmp(name, "MUL") == 0) {
        return mul;
    }
    if (strcmp(name, "DIV") == 0) {
        return divide_int;
    }
    return NULL;
}
