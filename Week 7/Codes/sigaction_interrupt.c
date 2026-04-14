// 2026 S1C COMP2017 Week 7 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Demonstration for `sigaction()` and signal handlers.


#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MSG "Caught SIGINT\n"

void sigint_handler(int signo, siginfo_t *info, void *context) {
    (void)signo;
    (void)info;
    (void)context;
    write(STDOUT_FILENO, MSG, sizeof(MSG) - 1);
}

int main(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);

    sa.sa_sigaction = sigint_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    while (1) {
        pause();
    }
}
