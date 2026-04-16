// 2026 S1C COMP2017 Week 7 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Tiny Delegator: main.c, ops.c, ops.h, program.c, program.h


#ifndef PROGRAM_H
#define PROGRAM_H

#include "ops.h"

typedef struct instruction {
    binop_fn op;
    int lhs;
    int rhs;
    int lhs_is_prev;
    int rhs_is_prev;
    struct instruction *next;
} instruction;

instruction *parse_line(const char *line);
void append_instruction(instruction **head, instruction **tail, instruction *ins);
int execute_program(instruction *head);
void free_program(instruction *head);

#endif
