## COMP2017 2026 S1 Week 10 Tutorial B

<table><tbody>
  <tr><td><b>Tutor</b></td><td>Hao Ren</td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au">hao.ren@sydney.edu.au</a></td></tr>
</tbody></table>

[TOC]

---

### B.1 Threads

A thread is a path of execution inside a process. Every running process has at least one thread. That first thread is often called the main thread. When we write a normal C program like this:

```c
int main(void) {
    printf("hello\n");
    return 0;
}
```

we are already using one thread. There is one line of execution: start at `main`, run statements in order, then exit.

A multi-threaded program has multiple execution paths inside the same process:

```text
one process
one virtual address space
multiple threads running inside it
```

Each thread can run independently, but the threads share the same process memory.

#### B.1.1 What a Thread Actually Is

A thread is an execution context that the operating system can schedule. For a thread to run, the CPU needs to know things like:

```text
which instruction should run next
what values are currently in registers
where this thread's stack is
```

So each thread has its own execution state. The most important thread-private pieces are:

```text
thread ID
stack
registers
instruction pointer / program counter
```

This is what allows one thread to be paused and another thread to run. The operating system can save one thread's CPU state and restore another thread's CPU state.

#### B.1.2 The Main Thread

When a process starts, it begins with one thread. That thread starts executing the program, eventually reaching:

```c
main()
```

So in a normal single-threaded program:

```text
process starts
main thread runs main()
main thread returns
process exits
```

If the program creates more threads, then there are multiple execution paths inside the same process.

For example:

```text
main thread       handles setup
worker thread 1   handles task A
worker thread 2   handles task B
worker thread 3   handles task C
```

They are all part of the same process.

#### B.1.3 Thread ID

Each thread has an identifier. In POSIX threads, the thread ID type is `pthread_t`.

For example:

```c
pthread_t thread;
```

This is different from a process ID:

```c
pid_t pid;
```

A useful distinction is:

```text
PID        identifies a process
pthread_t  identifies a thread within a POSIX threads program
```

On Linux, threads also have kernel-level thread IDs, but for our COMP2017-level pthread programming, `pthread_t` is the main type we usually use.

#### B.1.4 Stack

Each thread has its own stack.

The stack stores things like:

```text
function call frames
local variables
return addresses
temporary execution state
```

For example:

```c
void work(void) {
    int x = 10;
    printf("%d\n", x);
}
```

If two threads both call `work()`, each thread gets its own `x` because `x` is a local variable on that thread's stack.

So local variables inside functions are usually separate for each thread.

> [!CAUTION]
> The stack is separate, but it is not protected from other threads. Since all threads share the same virtual address space, another thread could access a stack variable if it somehow gets a pointer to it. That is usually a bad idea unless the lifetime is very carefully controlled.

#### B.1.5 Registers

Each thread has its own saved register state.

Registers are small storage locations inside the CPU. They hold values currently being used by the running code. For example, when one thread is running, the CPU registers contain that thread's values. When the OS switches to another thread, it saves the first thread’s registers and restores the second thread's registers. This is part of a context switch.

Conceptually:

```text
thread A runs
OS saves thread A's registers
OS restores thread B's registers
thread B runs
```

This is why each thread can continue from where it left off.

#### B.1.6 Instruction Pointer / Program Counter

Each thread has its own instruction pointer, also called the program counter. This tells the CPU which instruction should execute next. Different threads in the same process can be at different parts of the program at the same time. For example:

```text
thread 1 is inside read()
thread 2 is inside calculate()
thread 3 is inside printf()
```

They share the same program code, but each thread has its own current position in that code.

#### B.1.7 What Threads Share

Threads in the same process share the process's virtual address space. That means they share:

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

This is the major difference between threads and processes.

If two threads access the same global variable:

```c
int counter = 0;
```

then they are accessing the same `counter`.

If one thread does:

```c
counter = 42;
```

another thread can see:

```c
counter == 42
```

This makes communication between threads easy, but it also makes race conditions easy.

**Please also note Global variables are shared across threads in the same process, not across separate processes.**

#### B.1.8 Example: Shared Global Variable

```c
#include <pthread.h>
#include <stdio.h>

int shared = 0;

void *worker(void *arg) {
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

The worker thread changed the global variable, and the main thread saw the change. That works because both threads are inside the same process.

#### B.1.9 Example: Local Variables Are Separate

```c
#include <pthread.h>
#include <stdio.h>

void *worker(void *arg) {
    int id = *(int *)arg;
    int local = id * 10;

    printf("thread %d local = %d\n", id, local);
    return NULL;
}

int main(void) {
    pthread_t t1;
    pthread_t t2;

    int id1 = 1;
    int id2 = 2;

    pthread_create(&t1, NULL, worker, &id1);
    pthread_create(&t2, NULL, worker, &id2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return 0;
}
```

Each thread has its own `local` variable because `local` is on that thread's stack.

The output order is not guaranteed:

```text
thread 2 local = 20
thread 1 local = 10
```

or:

```text
thread 1 local = 10
thread 2 local = 20
```

Both are valid because the threads run concurrently.

#### B.1.10 Creating a Thread

In POSIX C programming, a thread is commonly created with `pthread_create()`.

The basic shape is:

```c
pthread_t thread;

pthread_create(&thread, NULL, function, argument);
```

The function must have this shape:

```c
void *function(void *arg);
```

Example:

```c
void *worker(void *arg) {
    printf("hello from worker thread\n");
    return NULL;
}
```

Then:

```c
pthread_create(&thread, NULL, worker, NULL);
```

This creates a new thread that starts running `worker`.

#### B.1.11 Waiting for a Thread

The parent-like idea for threads is `pthread_join()`

Example:

```c
pthread_join(thread, NULL);
```

This means:

```text
wait until this thread finishes
clean up its thread resources
```

This is similar in spirit to `waitpid()` for child processes, but for threads.

A thread can finish by returning from its start function:

```c
return NULL;
```

or by calling:

```c
pthread_exit(NULL);
```

#### B.1.12 Joinable vs Detached Threads

A normal thread is joinable.

That means another thread should eventually call:

```c
pthread_join(thread, NULL);
```

If we do not need to join a thread, we can detach it:

```c
pthread_detach(thread);
```

A detached thread cleans up its own resources when it finishes.

The distinction is:

```text
joinable thread   another thread collects its result with pthread_join()
detached thread   no join; resources are cleaned automatically on exit
```

#### B.1.13 Threads and Scheduling

Threads are scheduled by the operating system.

On a single CPU core, only one thread runs at a time, but the OS switches between them quickly:

```text
thread A runs briefly
thread B runs briefly
thread A runs again
thread C runs briefly
```

This gives concurrency.

On a multi-core CPU, multiple threads can run at the same time on different cores. That gives parallelism.

The distinction is:

```text
concurrency  multiple tasks make progress over time
parallelism  multiple tasks run at the same time
```

Threads can give concurrency. They can give true parallelism when the machine has multiple cores and the workload allows it.

#### B.1.14 Threads vs Processes

Processes are isolated. A process has its own virtual address space. If two processes both have a global variable called `x`, they are usually separate variables. Changing one does not automatically change the other. Processes communicate using explicit mechanisms such as:

```text
pipes
sockets
signals
shared memory
files
```

However, threads share memory. Threads inside the same process share the same virtual address space. So if thread A changes a global variable, thread B can see the change. This makes threads useful when multiple execution paths need to work on the same data. For example:

```text
one thread receives network messages
one thread processes them
one thread writes results to disk
```

They can all access shared queues or shared data structures in the same process. But shared memory also means bugs can affect the whole process. If one thread corrupts the heap, crashes, or writes through a bad pointer, the entire process can be affected.

#### B.1.15 Race Conditions

Because threads share memory, timing matters.

This code looks simple:

```c
counter++;
```

But it is not actually one indivisible operation. It is more like:

```text
read counter
add 1
write counter back
```

If two threads do this at the same time, the operations can interleave.

```text
counter starts at 0

thread A reads counter = 0
thread B reads counter = 0
thread A writes counter = 1
thread B writes counter = 1
```

The final value is:

```text
1
```

not:

```text
2
```

One update was lost. This is a race condition. When multiple threads access shared data and at least one thread writes, we usually need synchronization. Common synchronization tools include:

```text
mutexes
condition variables
semaphores
atomic operations
```

#### B.1.16 Heap Memory and Threads

The heap is shared between threads. If one thread allocates memory:

```c
int *p = malloc(sizeof *p);
```

another thread can use that memory if it receives the pointer.

Example:

```c
int *shared_ptr = malloc(sizeof *shared_ptr);
*shared_ptr = 42;
```

If `shared_ptr` is shared with another thread, that other thread can read or write the same heap allocation.

This is useful, but ownership matters. Important questions are:

```text
which thread is allowed to write?
which thread frees the memory?
can another thread still use it after it is freed?
```

A common bug is one thread freeing memory while another thread still has a pointer to it.

#### B.1.17 File Descriptors and Threads

Threads also share file descriptors because file descriptors belong to the process.

If one thread opens a file:

```c
int fd = open("data.txt", O_RDONLY);
```

another thread in the same process can use the same `fd` if it has access to the variable or the value is passed to it.

This also means one thread can affect another thread by closing a shared file descriptor:

```c
close(fd);
```

If another thread later tries to read from that `fd`, it may fail or accidentally refer to a newly reused file descriptor.

So file descriptor ownership also needs care in threaded programs.

#### B.1.18 Threads Are Not Fully Independent Programs

A thread is not the same as a process. A thread does not have its own independent address space. It does not have its own separate heap. It does not have its own separate globals. A thread is more like another execution path inside the same program instance.

This is why threads are lighter than processes, but also less isolated.

If a thread calls:

```c
exit(1);
```

the whole process exits, not just that thread. If a thread returns from its start function, only that thread ends.

---

### B.2

---

### B.3

---

### B.4

---

### B.5

---

### B.6
