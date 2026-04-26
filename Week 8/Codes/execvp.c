// 2026 S1C COMP2017 Week 8 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// `exec*()` series demo: `execvp()` example.


#include <stdio.h>
#include <unistd.h>

int main(void) {
    char *args[] = {"sort", "forkdemo.c", NULL};

    execvp("sort", args);

    perror("execvp failed");
    return 1;
}
