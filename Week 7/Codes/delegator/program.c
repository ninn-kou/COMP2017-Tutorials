// 2026 S1C COMP2017 Week 7 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Tiny Delegator: main.c, ops.c, ops.h, program.c, program.h


#include "program.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

instruction *parse_line(const char *line) {
    char opbuf[8];
    char lhsbuf[32];
    char rhsbuf[32];

    if (sscanf(line, "%7s %31s %31s", opbuf, lhsbuf, rhsbuf) != 3) {
        return NULL;
    }

    binop_fn op = parse_operation(opbuf);
    if (op == NULL) {
        return NULL;
    }

    instruction *ins = malloc(sizeof *ins);
    if (ins == NULL) {
        return NULL;
    }

    ins->op = op;
    ins->next = NULL;

    if (strcmp(lhsbuf, "%") == 0) {
        ins->lhs_is_prev = 1;
        ins->lhs = 0;
    } else {
        ins->lhs_is_prev = 0;
        ins->lhs = (int)strtol(lhsbuf, NULL, 10);
    }

    if (strcmp(rhsbuf, "%") == 0) {
        ins->rhs_is_prev = 1;
        ins->rhs = 0;
    } else {
        ins->rhs_is_prev = 0;
        ins->rhs = (int)strtol(rhsbuf, NULL, 10);
    }

    return ins;
}

void append_instruction(instruction **head, instruction **tail, instruction *ins) {
    if (*tail == NULL) {
        *head = ins;
        *tail = ins;
    } else {
        (*tail)->next = ins;
        *tail = ins;
    }
}

int execute_program(instruction *head) {
    int previous = 0;
    int have_previous = 0;

    for (instruction *cur = head; cur != NULL; cur = cur->next) {
        if ((cur->lhs_is_prev || cur->rhs_is_prev) && !have_previous) {
            fprintf(stderr, "%% used before any previous result\n");
            return 0;
        }

        int a = cur->lhs_is_prev ? previous : cur->lhs;
        int b = cur->rhs_is_prev ? previous : cur->rhs;

        if (cur->op == divide_int && b == 0) {
            fprintf(stderr, "Division by zero\n");
            return 0;
        }

        previous = cur->op(a, b);
        have_previous = 1;
        printf("%d\n", previous);
    }

    return 1;
}

void free_program(instruction *head) {
    while (head != NULL) {
        instruction *next = head->next;
        free(head);
        head = next;
    }
}
