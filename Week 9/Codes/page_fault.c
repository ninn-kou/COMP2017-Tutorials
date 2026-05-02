// 2026 S1C COMP2017 Week 9 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
// The goal is to deliberately create a memory region that exists in the process address space, but cannot be accessed.


#define _GNU_SOURCE

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>

static char *page;
static int page_size;
static volatile sig_atomic_t page_fixed = 0;

static void page_fault_handler_simulation(int sig, siginfo_t *info, void *uctx) {
    (void)sig;
    (void)uctx;

    const char *reason;

    if (info->si_code == SEGV_MAPERR) {
        reason = "address not mapped";
    } else if (info->si_code == SEGV_ACCERR) {
        reason = "permission denied";
    } else {
        reason = "unknown reason";
    }

    dprintf(STDERR_FILENO,
            "SIGSEGV at %p: %s\n",
            info->si_addr,
            reason);

    dprintf(STDERR_FILENO,
            "mmap returned page address: %p\n",
            page);

    if (info->si_addr == page) {
        dprintf(STDERR_FILENO,
                "faulting address matches the mmap page\n");
    }

    uintptr_t fault_addr = (uintptr_t)info->si_addr;
    uintptr_t page_start = (uintptr_t)page;
    uintptr_t page_end = page_start + (uintptr_t)page_size;

    if (fault_addr < page_start || fault_addr >= page_end || page_fixed) {
        dprintf(STDERR_FILENO,
                "unexpected fault; exiting\n");
        _exit(1);
    }

    if (mprotect(page, (size_t)page_size, PROT_READ | PROT_WRITE) == -1) {
        dprintf(STDERR_FILENO,
                "mprotect failed\n");
        _exit(1);
    }

    page_fixed = 1;

    dprintf(STDERR_FILENO,
            "page permission changed to read/write\n");

    /*
     * Do not call _exit() here.
     * Returning lets the original faulting instruction retry.
     */
}

int main(void) {
    struct sigaction sa = {0};

    sa.sa_sigaction = page_fault_handler_simulation;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    long size = sysconf(_SC_PAGESIZE);

    if (size == -1) {
        perror("sysconf");
        exit(1);
    }

    page_size = (int)size;

    page = mmap(NULL,
                (size_t)page_size,
                PROT_NONE,
                MAP_PRIVATE | MAP_ANONYMOUS,
                -1,
                0);

    if (page == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    dprintf(STDERR_FILENO,
            "mapped PROT_NONE page at %p\n",
            page);

    /*
     * This write triggers SIGSEGV.
     *
     * The page exists, but has PROT_NONE, so the handler should report
     * SEGV_ACCERR rather than SEGV_MAPERR.
     */
    page[0] = 42;

    /*
     * If the handler successfully changed the page to read/write,
     * the write above is retried and succeeds.
     */
    dprintf(STDERR_FILENO,
            "recovered: page[0] = %d\n",
            page[0]);

    munmap(page, (size_t)page_size);
    return 0;
}
