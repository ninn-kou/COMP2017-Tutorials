// 2026 S1C COMP2017 Week 7 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// A program takes 2 integers and divides 1 number by the other.
// Task is to catch that signal, output "You tried to divide by zero?" to
// STDERR, and exit failure.


#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#define DIV_ZERO_MSG "You tried to divide by zero?\n"

static void fpe_handler(int sig) {
    (void)sig;  // unused
    write(STDERR_FILENO, DIV_ZERO_MSG, sizeof(DIV_ZERO_MSG) - 1);
    _exit(EXIT_FAILURE);
}

int main(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = fpe_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGFPE, &sa, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    int a, b;
    printf("Enter two integers: ");
    fflush(stdout);

    if (scanf("%d %d", &a, &b) != 2) {
        fprintf(stderr, "Invalid input\n");
        return EXIT_FAILURE;
    }

    int result = a / b;
    printf("Result: %d\n", result);

    return EXIT_SUCCESS;
}
