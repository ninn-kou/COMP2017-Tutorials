// 2026 S1C COMP2017 Week 7 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Tiny Delegator: main.c, ops.c, ops.h, program.c, program.h


#include <stdio.h>
#include <string.h>

#include "program.h"

int main(void) {
    instruction *head = NULL;
    instruction *tail = NULL;
    char line[128];

    while (fgets(line, sizeof line, stdin) != NULL) {
        line[strcspn(line, "\n")] = '\0';

        if (line[0] == '\0') {
            continue;
        }

        instruction *ins = parse_line(line);
        if (ins == NULL) {
            fprintf(stderr, "Invalid instruction: %s\n", line);
            free_program(head);
            return 1;
        }

        append_instruction(&head, &tail, ins);
    }

    int ok = execute_program(head);
    free_program(head);
    return ok ? 0 : 1;
}
