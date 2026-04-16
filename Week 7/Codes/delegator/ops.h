// 2026 S1C COMP2017 Week 7 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Tiny Delegator: main.c, ops.c, ops.h, program.c, program.h


#ifndef OPS_H
#define OPS_H

typedef int (*binop_fn)(int, int);

int add(int a, int b);
int sub(int a, int b);
int mul(int a, int b);
int divide_int(int a, int b);

binop_fn parse_operation(const char *name);

#endif
