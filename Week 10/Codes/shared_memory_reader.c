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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_NAME "/comp2017_shared_example"

struct shared_data {
    int value;
};

int main(void) {
    int fd = shm_open(SHM_NAME, O_RDWR, 0600);

    if (fd == -1) {
        perror("shm_open");
        exit(1);
    }

    struct shared_data *data = mmap(NULL,
                                    sizeof *data,
                                    PROT_READ | PROT_WRITE,
                                    MAP_SHARED,
                                    fd,
                                    0);

    if (data == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    printf("reader saw value %d\n", data->value);

    munmap(data, sizeof *data);
    close(fd);

    return 0;
}
