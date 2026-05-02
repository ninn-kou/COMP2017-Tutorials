// 2026 S1C COMP2017 Week 9 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// Fork three children and let the parent track them using `SIGCHLD`.


#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUM_CHILDREN 3

static volatile sig_atomic_t children_left = NUM_CHILDREN;

static void write_str(const char *s) {
    size_t len = 0;

    while (s[len] != '\0') {
        len++;
    }

    write(STDOUT_FILENO, s, len);
}

static void write_num(long n) {
    char buf[32];
    int i = (int)sizeof buf;

    if (n == 0) {
        write(STDOUT_FILENO, "0", 1);
        return;
    }

    while (n > 0 && i > 0) {
        buf[--i] = (char)('0' + (n % 10));
        n /= 10;
    }

    write(STDOUT_FILENO, &buf[i], sizeof buf - (size_t)i);
}

static void handler(int sig, siginfo_t *info, void *uctx) {
    (void)sig;
    (void)uctx;

    int status;
    pid_t pid = waitpid(info->si_pid, &status, WNOHANG);

    if (pid <= 0) {
        return;
    }

    write_str("child ");
    write_num((long)info->si_pid);

    if (info->si_code == CLD_EXITED) {
        write_str(" exited normally with status ");
        write_num((long)info->si_status);
    } else if (info->si_code == CLD_KILLED) {
        write_str(" was killed by signal ");
        write_num((long)info->si_status);
    } else if (info->si_code == CLD_DUMPED) {
        write_str(" was killed by signal ");
        write_num((long)info->si_status);
        write_str(" and dumped core");
    } else {
        write_str(" changed state");
    }

    write_str("\n");
    children_left--;
}

int main(void) {
    struct sigaction sa;
    sigset_t block_set;
    sigset_t old_set;
    sigset_t wait_set;

    sigemptyset(&block_set);
    sigaddset(&block_set, SIGCHLD);

    if (sigprocmask(SIG_BLOCK, &block_set, &old_set) == -1) {
        perror("sigprocmask");
        exit(1);
    }

    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_NOCLDSTOP;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    int sleeps[NUM_CHILDREN] = {1, 2, 3};
    int statuses[NUM_CHILDREN] = {10, 20, 30};

    for (int i = 0; i < NUM_CHILDREN; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            exit(1);
        }

        if (pid == 0) {
            sleep(sleeps[i]);

            /*
             * Optional experiment:
             * Replace one child's normal exit with abort().
             *
             * if (i == 1) {
             *     abort();
             * }
             */

            _exit(statuses[i]);
        }

        /* Parent continues the loop to fork the next child. */
    }

    wait_set = old_set;
    sigdelset(&wait_set, SIGCHLD);

    while (children_left > 0) {
        sigsuspend(&wait_set);
    }

    if (sigprocmask(SIG_SETMASK, &old_set, NULL) == -1) {
        perror("sigprocmask");
        exit(1);
    }

    printf("all children reaped\n");
    return 0;
}
