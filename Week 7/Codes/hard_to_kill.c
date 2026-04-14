// 2026 S1C COMP2017 Week 7 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// A "Hard to Kill" program, require you press `Ctrl+C`, `Ctrl+Z` and `Ctrl+\`
// in that order before it quits. If you enters the sequence wrong, you need to
// start from the beginning again.


#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static volatile sig_atomic_t stage = 0;
static volatile sig_atomic_t tick = 0;

static const char MSG_STAGE1[] = "Kill Sequence Stage 1 Primed\n";
static const char MSG_STAGE2[] = "Kill Sequence Stage 2 Primed\n";
static const char MSG_ABORT[]  = "Kill Sequence Aborted\n";
static const char MSG_DONE[]   = "Kill Sequence Complete, Program Exit\n";

static void handle_signal(int sig) {
    int saved_errno = errno;

    if (sig == SIGALRM) {
        tick = 1;
        errno = saved_errno;
        return;
    }

    if (sig == SIGINT) {
        if (stage == 0) {
            stage = 1;
            write(STDOUT_FILENO, MSG_STAGE1, sizeof(MSG_STAGE1) - 1);
        } else {
            stage = 0;
            write(STDOUT_FILENO, MSG_ABORT, sizeof(MSG_ABORT) - 1);
        }
    } else if (sig == SIGTSTP) {
        if (stage == 1) {
            stage = 2;
            write(STDOUT_FILENO, MSG_STAGE2, sizeof(MSG_STAGE2) - 1);
        } else {
            stage = 0;
            write(STDOUT_FILENO, MSG_ABORT, sizeof(MSG_ABORT) - 1);
        }
    } else if (sig == SIGQUIT) {
        if (stage == 2) {
            write(STDOUT_FILENO, MSG_DONE, sizeof(MSG_DONE) - 1);
            _exit(EXIT_SUCCESS);
        } else {
            stage = 0;
            write(STDOUT_FILENO, MSG_ABORT, sizeof(MSG_ABORT) - 1);
        }
    }

    errno = saved_errno;
}

static int install_handler(int sig) {
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);

    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);

    /* Block these signals while any handler is running. */
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGTSTP);
    sigaddset(&sa.sa_mask, SIGQUIT);
    sigaddset(&sa.sa_mask, SIGALRM);

    return sigaction(sig, &sa, NULL);
}

int main(void) {
    if (install_handler(SIGINT) == -1 ||
        install_handler(SIGTSTP) == -1 ||
        install_handler(SIGQUIT) == -1 ||
        install_handler(SIGALRM) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    unsigned long long a = 0;
    unsigned long long b = 1;
    char buf[64];

    /* First Fibonacci number appears after 5 seconds. */
    alarm(5);

    for (;;) {
        pause();   /* Wakes up when any caught signal arrives. */

        if (tick) {
            tick = 0;

            int len = snprintf(buf, sizeof buf, "%llu\n", a);
            if (len > 0) {
                write(STDOUT_FILENO, buf, (size_t)len);
            }

            unsigned long long next = a + b;
            a = b;
            b = next;

            alarm(5);
        }
    }
}
