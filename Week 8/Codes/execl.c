// 2026 S1C COMP2017 Week 8 Tutorial B
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// `exec*()` series demo: `execl()` example.


#include <stdio.h>
#include <unistd.h>

int main(void) {
    puts("before exec");

    execl("/bin/echo", "echo", "hello", "world", (char *)NULL);

    puts("after exec");
    return 0;
}
