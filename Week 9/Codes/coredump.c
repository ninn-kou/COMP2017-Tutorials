// 2026 S1C COMP2017 Week 9 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// A program that installs a SIGSEGV handler using sigaction with SA_SIGINFO. The handler should print:
// 1. the memory address that caused the fault,
// 2. whether the address was unmapped or mapped with the wrong permissions.


#define _GNU_SOURCE

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

static void handler(int sig, siginfo_t *info, void *uctx) {
    (void)sig;
    (void)uctx;

    const char *reason;

    if (info->si_code == SEGV_MAPERR) {
        reason = "address not mapped";
    } else if (info->si_code == SEGV_ACCERR) {
        reason = "permission denied";
    } else {
        reason = "unknown SIGSEGV reason";
    }

    // Tips from your tutor:
    // It would be better to use write() instead of dprintf() in your own implementation.
    dprintf(STDERR_FILENO,
            "SIGSEGV at %p: %s\n",
            info->si_addr,
            reason);

    _exit(1);
}

int main(void) {
    struct sigaction sa;

    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;

    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    /*
     * Experiment 1:
     * NULL dereference.
     *
     * This usually produces SEGV_MAPERR because address 0 is not mapped.
     */
    int *p = NULL;
    *p = 42;

    /*
     * Experiment 2:
     * Write to a read-only mmap'd page.
     *
     * Comment out Experiment 1 above first, otherwise the program exits
     * before reaching this code.
     *
     * This usually produces SEGV_ACCERR because the page exists, but it
     * does not have write permission.
     */
    /*
    long page_size = sysconf(_SC_PAGESIZE);

    if (page_size == -1) {
        perror("sysconf");
        exit(1);
    }

    char *page = mmap(NULL,
                      (size_t)page_size,
                      PROT_READ,
                      MAP_PRIVATE | MAP_ANONYMOUS,
                      -1,
                      0);

    if (page == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    page[0] = 'x';
    */

    return 0;
}
