// 2026 S1C COMP2017 Week 10 Tutorial A
// Tutor: Hao Ren (hao.ren@sydney.edu.au)
//
// Demonstrates the use of shared memory.
//
// Compile both the writer and reader source files, then run:
//     ./shared_memory_writer
//     ./shared_memory_reader
//     ./shared_memory_cleanup


#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <sys/mman.h>

#define SHM_NAME "/comp2017_shared_example"

int main(void) {
    if (shm_unlink(SHM_NAME) == -1) {
        perror("shm_unlink");
        return 1;
    }

    return 0;
}
