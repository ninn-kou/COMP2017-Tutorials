// 2026 S1C COMP2017 Week 8 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// `exec*()` series demo: `execv()` example.


#include <stdio.h>
#include <unistd.h>

int main(void) {
    char *args[] = {"echo", "hi", "there", NULL};

    execv("/bin/echo", args);

    perror("execv failed");
    return 1;
}
