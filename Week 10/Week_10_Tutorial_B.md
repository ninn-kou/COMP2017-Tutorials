## COMP2017 2026 S1 Week 10 Tutorial B

<table><tbody>
  <tr><td><b>Tutor</b></td><td>Hao Ren</td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au">hao.ren@sydney.edu.au</a></td></tr>
</tbody></table>

[TOC]

---

### B.1 Threads and POSIX Threads (`pthreads`)

A **thread** is a path of execution inside a process. Every process starts with at least one thread, called the **main thread**. In a normal C program, the main thread starts running the program and eventually reaches `main()`:

```c
int main(void) {
    printf("hello\n");
    return 0;
}
```

A multi-threaded program has multiple execution paths inside the same process:

```text
one process
one virtual address space
multiple threads running inside it
```

Threads are lighter than processes because they share the same process memory, but this also means one thread can affect the others. If one thread corrupts shared memory or calls `exit()`, the whole process can be affected.

Each thread has its own execution state. The most important thread-private parts are:

```text
thread ID
stack
registers
instruction pointer / program counter
```

The stack stores function call frames, local variables, return addresses, and temporary execution state. If two threads call the same function, each thread gets its own local variables on its own stack:

```c
void work(void) {
    int x = 10;   // each thread gets its own x
    printf("%d\n", x);
}
```

Each thread also has its own saved register state and instruction pointer. This lets the operating system pause one thread and resume another:

```text
thread A runs
OS saves thread A's registers and program counter
OS restores thread B's registers and program counter
thread B continues from where it stopped
```

Threads in the same process share:

```text
program code
global variables
static variables
heap memory
mapped memory regions
open file descriptors
current working directory
process environment
```

This is the key difference from processes. Separate processes have separate virtual address spaces, so global variables are usually not shared between them. Threads inside the same process do share globals, heap memory, and file descriptors.

```text
processes communicate using pipes, sockets, signals, shared memory, files, etc.
threads can communicate directly through shared memory inside the same process
```

Global variables are shared across threads in the same process, not across separate processes.

#### B.1.1 Creating and Joining POSIX Threads

`pthreads` is the POSIX thread API for C. To use it, include:

```c
#include <pthread.h>
```

Compile with `-pthread`:

```bash
gcc -std=c11 -Wall -Wextra -pedantic -g program.c -o program -pthread
```

The `-pthread` flag matters because it enables pthread support during compilation and linking.

A new thread starts by running a function. That function must have this shape:

```c
void *function_name(void *arg);
```

Example worker function:

```c
static void *worker(void *arg) {
    (void)arg;
    printf("hello from worker thread\n");
    return NULL;
}
```

Create a thread with `pthread_create()`:

```c
pthread_t thread;

int err = pthread_create(&thread, NULL, worker, NULL);
```

The prototype is:

```c
int pthread_create(pthread_t *thread,
                   const pthread_attr_t *attr,
                   void *(*start_routine)(void *),
                   void *arg);
```

The arguments are:

```text
thread          where the new thread ID is stored
attr            thread attributes; usually NULL for defaults
start_routine   function the new thread will run
arg             argument passed to that function
```

`pthread_create()` returns `0` on success. On failure, it returns an error number. Many pthread functions return the error number directly, so use `strerror(err)` instead of assuming `errno` was set.

Wait for a joinable thread with `pthread_join()`:

```c
int err = pthread_join(thread, NULL);
```

This means:

```text
wait until the thread finishes
discard its return value
clean up its joinable thread resources
```

A normal pthread is **joinable** by default. A joinable thread should be joined exactly once. If we do not need to join a thread, we can detach it:

```c
pthread_detach(thread);
```

A detached thread cannot be joined; its resources are cleaned up automatically when it finishes. For tutorial code, joinable threads are usually clearer.

Simple example:

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void *worker(void *arg) {
    (void)arg;

    printf("hello from worker thread\n");
    return NULL;
}

int main(void) {
    pthread_t thread;

    int err = pthread_create(&thread, NULL, worker, NULL);

    if (err != 0) {
        fprintf(stderr, "pthread_create: %s\n", strerror(err));
        return EXIT_FAILURE;
    }

    printf("hello from main thread\n");

    err = pthread_join(thread, NULL);

    if (err != 0) {
        fprintf(stderr, "pthread_join: %s\n", strerror(err));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
```

Possible output:

```text
hello from main thread
hello from worker thread
```

or:

```text
hello from worker thread
hello from main thread
```

Both are valid because the threads run concurrently and the output order is not guaranteed.

#### B.1.2 Passing Data to Threads

The thread function receives one argument:

```c
void *arg
```

Because it is a `void *`, we usually pass a pointer and cast it back inside the thread function.

```c
static void *worker(void *arg) {
    int id = *(int *)arg;

    printf("thread %d is running\n", id);
    return NULL;
}
```

The pointer must still be valid when the thread uses it. This is safe:

```c
pthread_t thread;
int id = 7;

pthread_create(&thread, NULL, worker, &id);
pthread_join(thread, NULL);
```

The variable `id` is still alive because `main()` waits for the worker before returning.

A common bug is passing the address of a loop variable:

```c
for (int i = 0; i < 5; i++) {
    pthread_create(&threads[i], NULL, worker, &i);   // wrong
}
```

All threads receive the address of the same variable `i`. By the time a thread reads it, `i` may have changed.

Use a separate array instead:

```c
int ids[5];

for (int i = 0; i < 5; i++) {
    ids[i] = i;
    pthread_create(&threads[i], NULL, worker, &ids[i]);
}
```

When creating multiple threads, usually create all threads first, then join all threads:

```text
create all threads
join all threads
```

If we create and immediately join inside the same loop, the program mostly runs one worker at a time.

Full example:

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NTHREADS 5

static void *worker(void *arg) {
    int id = *(int *)arg;

    printf("hello from thread %d\n", id);
    return NULL;
}

int main(void) {
    pthread_t threads[NTHREADS];
    int ids[NTHREADS];

    for (int i = 0; i < NTHREADS; i++) {
        ids[i] = i;

        int err = pthread_create(&threads[i], NULL, worker, &ids[i]);

        if (err != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(err));
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < NTHREADS; i++) {
        int err = pthread_join(threads[i], NULL);

        if (err != 0) {
            fprintf(stderr, "pthread_join: %s\n", strerror(err));
            return EXIT_FAILURE;
        }
    }

    printf("all threads finished\n");
    return EXIT_SUCCESS;
}
```

Possible output:

```text
hello from thread 0
hello from thread 3
hello from thread 1
hello from thread 4
hello from thread 2
all threads finished
```

The thread output order is not guaranteed.

#### B.1.3 Returning Values and Exiting Threads

A thread function returns a `void *`. The joining thread can collect that return value using the second argument of `pthread_join()`.

```c
static void *worker(void *arg) {
    (void)arg;

    int *result = malloc(sizeof *result);

    if (result == NULL) {
        return NULL;
    }

    *result = 42;
    return result;
}
```

The main thread can collect and free the result:

```c
void *retval;

pthread_join(thread, &retval);

int *result = retval;

if (result != NULL) {
    printf("result = %d\n", *result);
    free(result);
}
```

Do not return the address of a local variable:

```c
static void *worker(void *arg) {
    int result = 42;
    return &result;   // wrong
}
```

`result` is on the worker thread's stack. Once the function returns, that stack frame is gone.

A worker thread normally exits by returning from its start function:

```c
return NULL;
```

It can also call:

```c
pthread_exit(NULL);
```

Both end the current thread. However, `exit(0)` ends the whole process, not just the current thread.

```text
return from worker    ends that worker thread
pthread_exit()        ends the calling thread
exit()                ends the entire process
```

If the main thread calls `pthread_exit(NULL)`, the main thread ends but the process can keep running while other threads continue. If `main()` simply returns, that is equivalent to calling `exit()`, so the whole process exits. For most beginner programs, prefer `pthread_join()` because it makes the program lifetime clearer.

#### B.1.4 Shared Data, Race Conditions, and Thread Safety

Threads share memory, so a worker thread can change a global variable and the main thread can see the change:

```c
#include <pthread.h>
#include <stdio.h>

int shared = 0;

static void *worker(void *arg) {
    (void)arg;

    shared = 100;
    return NULL;
}

int main(void) {
    pthread_t t;

    pthread_create(&t, NULL, worker, NULL);
    pthread_join(t, NULL);

    printf("shared = %d\n", shared);
    return 0;
}
```

Possible output:

```text
shared = 100
```

Local variables are usually separate because each thread has its own stack:

```c
static void *worker(void *arg) {
    int id = *(int *)arg;
    int local = id * 10;

    printf("thread %d local = %d\n", id, local);
    return NULL;
}
```

If two threads both call `worker()`, each thread has its own `local` variable.

The heap is shared between threads. If one thread allocates memory with `malloc()`, another thread can use that memory if it receives the pointer. This is useful, but ownership must be clear:

```text
which thread is allowed to write?
which thread frees the memory?
can another thread still use it after it is freed?
```

File descriptors are also shared because file descriptors belong to the process. If one thread closes an fd while another thread is still using it, the other thread may fail or accidentally use a newly reused fd number.

Because threads share memory, timing matters. This code looks simple:

```c
counter++;
```

But it is really more like:

```text
read counter
add 1
write counter back
```

If two threads do this at the same time, their operations can interleave:

```text
counter starts at 0

thread A reads counter = 0
thread B reads counter = 0
thread A writes counter = 1
thread B writes counter = 1
```

The final value is `1`, not `2`. One update was lost. This is a **race condition**.

When multiple threads access shared data and at least one thread writes, we usually need synchronization, such as:

```text
mutexes
condition variables
semaphores
atomic operations
```

#### B.1.5 Useful Comparisons and Common Mistakes

Threads are scheduled by the operating system. On a single CPU core, only one thread runs at a time, but the OS switches between them quickly. On a multi-core CPU, multiple threads can run at the same time on different cores.

```text
concurrency  multiple tasks make progress over time
parallelism  multiple tasks run at the same time
```

Useful comparisons:

```text
fork()             creates a new process
pthread_create()   creates a new thread inside the same process

waitpid()          waits for a child process
pthread_join()     waits for a thread

PID                identifies a process
pthread_t          identifies a POSIX thread
```

Common mistakes:

```text
Forgetting to compile with -pthread.
Using the wrong thread function signature.
Ignoring pthread_create() or pthread_join() return values.
Joining immediately after each pthread_create(), which removes most concurrency.
Passing &i from a loop to all threads.
Returning a pointer to a local variable.
Calling exit() when only the current thread should end.
Accessing shared data without synchronization.
Closing a file descriptor while another thread may still use it.
```

Short summary:

```text
A thread is an execution path inside a process.
POSIX threads are created with pthread_create().
Joinable threads are waited for with pthread_join().
Threads share globals, heap memory, and file descriptors.
Each thread has its own stack, registers, and program counter.
Shared data needs synchronization when at least one thread writes.
```

---

### B.3

---

### B.4

---

### B.5

---

### B.6
