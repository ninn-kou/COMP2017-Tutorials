## COMP2017 2026 S1 Week 10 Tutorial B

<table><tbody>
  <tr><td><b>Tutor</b></td><td>Hao Ren</td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au">hao.ren@sydney.edu.au</a></td></tr>
</tbody></table>

- [COMP2017 2026 S1 Week 10 Tutorial B](#comp2017-2026-s1-week-10-tutorial-b)
  - [B.1 Threads and POSIX Threads (`pthreads`)](#b1-threads-and-posix-threads-pthreads)
    - [B.1.1 Creating and Joining POSIX Threads](#b11-creating-and-joining-posix-threads)
    - [B.1.2 Passing Data to Threads](#b12-passing-data-to-threads)
    - [B.1.3 Returning Values and Exiting Threads](#b13-returning-values-and-exiting-threads)
    - [B.1.4 Shared Data, Race Conditions, and Thread Safety](#b14-shared-data-race-conditions-and-thread-safety)
    - [B.1.5 Useful Comparisons and Common Mistakes](#b15-useful-comparisons-and-common-mistakes)
  - [B.2 Exercise: Hello From Threads](#b2-exercise-hello-from-threads)
    - [B.2.1 Step 1: Write the Thread Function](#b21-step-1-write-the-thread-function)
    - [B.2.2 Step 2: Create the Threads](#b22-step-2-create-the-threads)
    - [B.2.3 Step 3: Join the Threads](#b23-step-3-join-the-threads)
    - [B.2.4 What Happens If We Increase to 20 Threads and Remove `pthread_join()`?](#b24-what-happens-if-we-increase-to-20-threads-and-remove-pthread_join)
  - [B.3 Exercise: Passing Arguments](#b3-exercise-passing-arguments)
    - [B.3.1 Step 1: Give Each Thread a Stable Argument](#b31-step-1-give-each-thread-a-stable-argument)
    - [B.3.2 Step 2: Fix the Thread Function Return Value](#b32-step-2-fix-the-thread-function-return-value)
  - [B.4 Exercise: Summation with Threads](#b4-exercise-summation-with-threads)
    - [B.4.1 Step 1: Define a Thread Task Struct](#b41-step-1-define-a-thread-task-struct)
    - [B.4.2 Step 2: Split the Work Evenly](#b42-step-2-split-the-work-evenly)
    - [B.4.3 Step 3: Join Threads and Combine Partial Sums](#b43-step-3-join-threads-and-combine-partial-sums)
  - [B.5 Exercise: Kitchen of Hell](#b5-exercise-kitchen-of-hell)

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

### B.2 Exercise: Hello From Threads

The goal is to create multiple threads. Each thread runs the same function and prints:

```text
Hello World
```

> [!IMPORTANT]
> Refer to [`hello.c`](./Codes/hello.c) for the solution to this exercise.
> You need to use the `-pthread` flag when compiling.

Unlike `fork()`, a new thread does not continue from the current line as a separate process. A pthread starts by calling the function we pass to `pthread_create()`.

#### B.2.1 Step 1: Write the Thread Function

A pthread start function must have this shape:

```c
void *function_name(void *arg)
```

For this exercise, the thread does not need an argument, so we can ignore `arg`:

```c
static void *print_hello(void *arg) {
    (void)arg;

    printf("Hello World\n");
    return NULL;
}
```

The return value is `NULL` because the main thread does not need to collect a result.

#### B.2.2 Step 2: Create the Threads

We store the thread IDs in an array:

```c
pthread_t threads[NTHREADS];
```

Then create each thread:

```c
// &threads[i]   where to store the new thread ID
// NULL          use default thread attributes
// print_hello   function the thread should run
// NULL          argument passed to print_hello

pthread_create(&threads[i], NULL, print_hello, NULL);
```

#### B.2.3 Step 3: Join the Threads

After creating all the threads, the main thread should wait for them:

```c
pthread_join(threads[i], NULL);
```

This matters because `pthread_join()` lets the main thread wait until each worker thread has finished. Without joining, `main()` may return before the worker threads get a chance to print.

#### B.2.4 What Happens If We Increase to 20 Threads and Remove `pthread_join()`?

Change:

```c
#define NTHREADS 20
```

Then comment out the join loop:

```c
/*
for (int i = 0; i < NTHREADS; i++) {
    pthread_join(threads[i], NULL);
}
*/
```

Now the program may print fewer than 20 lines, or sometimes no lines at all.

**Why?** Because `main()` can finish before the worker threads finish. When `main()` returns, the whole process exits. Since all threads live inside the same process, exiting the process kills the remaining threads too.

---

### B.3 Exercise: Passing Arguments

The goal is to create 10 threads and have each thread print its own thread number:

```text
Hello From Thread 0!
Hello From Thread 1!
...
Hello From Thread 9!
```

The program is close, but there is one major bug:

```c
pthread_create(threads + i, NULL, routine, &i);
```

Every thread receives the address of the same loop variable `i`.

**What goes wrong?**

This line passes a pointer `&i` to `i`:


But there is only one `i` variable in the loop. So all threads receive the same address:

```text
thread 0 gets &i
thread 1 gets &i
thread 2 gets &i
...
thread 9 gets &i
```

The threads do not necessarily run immediately. By the time a thread reads `*id`, the loop may have already changed `i`. So instead of each thread printing its own number, we might see output like:

```text
Hello From Thread 3!
Hello From Thread 7!
Hello From Thread 10!
Hello From Thread 10!
Hello From Thread 10!
```

The exact result changes depending on scheduling. This is a race condition involving the shared loop variable.

> [!IMPORTANT]
> Refer to [`args.c`](./Codes/args.c) for the solution to this exercise.
> You need to use the `-pthread` flag when compiling.

#### B.3.1 Step 1: Give Each Thread a Stable Argument

Instead of passing `&i`, create an array of IDs:

```c
int ids[NTHREADS];
```

Then each thread gets a pointer to its own array element:

```c
ids[i] = i;
pthread_create(&threads[i], NULL, routine, &ids[i]);
```

Now the addresses are different:

```text
thread 0 gets &ids[0]
thread 1 gets &ids[1]
thread 2 gets &ids[2]
...
```

Each value remains valid until after all threads have joined, because `ids` lives in `main()`.

#### B.3.2 Step 2: Fix the Thread Function Return Value

The thread function has type:

```c
void *routine(void *args)
```

So it should return a `void *`.

This is missing in the original code:

```c
return NULL;
```

Returning `NULL` means the thread finished normally and does not return any useful result to `pthread_join()`.

Also, the output order should not be expected to be `0, 1, 2, ...`. Thread scheduling is controlled by the operating system, so the print order can change every run.

---

### B.4 Exercise: Summation with Threads

We need to add up all numbers in a large array using a user-chosen number of threads. The array is created by:

```c
int *large_array = get_nums();
```

Because `get_nums()` uses `malloc()`, the returned array must eventually be freed by `free(large_array);` to avoid any memory leak.

In addition, do not have all threads update one shared `sum` directly.

This would be unsafe:

```c
sum += large_array[i];
```

if multiple threads do it at the same time, because `sum += x` is not atomic. Instead, each thread stores its own result:

```c
tasks[i].partial_sum
```

Then the main thread combines the results after all threads have finished. This avoids needing a mutex.

> [!IMPORTANT]
> Refer to [`sum.c`](./Codes/sum.c) for the solution to this exercise.
> You need to use the `-pthread` flag when compiling.

#### B.4.1 Step 1: Define a Thread Task Struct

Each thread needs to know:

```text
which array to read
where its range starts
where its range ends
where to store its partial sum
```

So we use:

```c
typedef struct {
    int *arr;
    int start;
    int end;
    long long partial_sum;
} ThreadTask;
```

The range is `[start, end)` meaning `start` is included and `end` is not included.

#### B.4.2 Step 2: Split the Work Evenly

If:

```text
SIZE = 10000
nthreads = 3
```

then the work does not divide perfectly.

So we use:

```c
base = SIZE / nthreads;
rem  = SIZE % nthreads;
```

The first `rem` threads get one extra element. This gives balanced ranges.

---

#### B.4.3 Step 3: Join Threads and Combine Partial Sums

After creating all threads:

```c
pthread_join(threads[i], NULL);
```

Then main adds:

```c
sum += tasks[i].partial_sum;
```

This is safe because each thread has already finished writing its partial sum.

Then don't forget about `free()`. The main memory leak is the array returned by `get_nums()`. Since it was allocated with `malloc()`, it must be released with `free()`.

---

### B.5 Exercise: Kitchen of Hell

We need several threads:

```text
Rordon thread      keeps asking for the lamb sauce
Head chef thread   creates and waits for the sous chefs
Sous chef threads  prepare the components
```

There are 3 sous chefs:

```text
Red Wine Chef
Lamb Stock Chef
Herb Seasoning Chef
```

Each sous chef prepares its component `NUM_MEALS` times. Once all components are ready, the head chef prints:

```text
Head Chef: All lamb sauce ready!
```

While the lamb sauce is not ready, Rordon prints once per second:

```text
Rordon: WHERE IS THE LAMB SAUCE
```

> [!IMPORTANT]
> Refer to [`kitchen.c`](./Codes/kitchen.c) for the solution to this exercise.
> You need to use the `-pthread` flag when compiling.

The head chef should create all 3 sous chef threads first, then join them.

This allows the sous chefs to work concurrently:

```text
create Red Wine chef
create Lamb Stock chef
create Herb Seasoning chef
then wait for all of them
```

Do not create and immediately join one sous chef at a time, because that would make the work mostly sequential.

> [!NOTE]
> One small correctness fix: the scaffold uses a shared `int meals_ready`. In real pthread code, one thread writing a plain `int` while another thread reads it is a data race. Since this exercise does not want locks yet, we can use a small atomic flag instead:
>
> ```c
> static atomic_int meals_ready = 0;
> ```
>
> his keeps the code simple without introducing mutexes.

For our solution, each sous chef receives a `SousChefTask`:

```c
typedef struct {
    const char *component;
    void (*prep)(void);
    int *count;
} SousChefTask;
```

This tells the sous chef:

```text
what component to prepare
which preparation function to call
which counter to update
```

The head chef creates all sous chef threads:

```c
pthread_create(&sous_chefs[i], NULL, sous_chef, &tasks[i]);
```

Then waits for all of them:

```c
pthread_join(sous_chefs[i], NULL);
```

Only after all sous chefs are joined does the head chef print:

```text
Head Chef: All lamb sauce ready!
```

The Rordon thread creates the head chef thread, then keeps checking:

```c
atomic_load(&meals_ready)
```

until the head chef marks the meals as ready.
