## COMP2017 2026 S1 Week 11 Tutorial A

<table><tbody>
  <tr><td><b>Tutor</b></td><td>Hao Ren</td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au">hao.ren@sydney.edu.au</a></td></tr>
</tbody></table>

- [COMP2017 2026 S1 Week 11 Tutorial A](#comp2017-2026-s1-week-11-tutorial-a)
  - [A.1 Critical Sections](#a1-critical-sections)
    - [A.1.1 Identifying the Critical Section](#a11-identifying-the-critical-section)
    - [A.1.2 Shared Data vs Local Data](#a12-shared-data-vs-local-data)
    - [A.1.3 `volatile`](#a13-volatile)
  - [A.2 Mutexes](#a2-mutexes)
  - [A.3 Mutex Initialisation and Attributes](#a3-mutex-initialisation-and-attributes)
  - [A.4 Exercise: Joey Doesn't Share Food](#a4-exercise-joey-doesnt-share-food)
  - [A.5 Exercise: Concurrent Linked List in C](#a5-exercise-concurrent-linked-list-in-c)
    - [A.5.1 Step 1: Extend the Structs](#a51-step-1-extend-the-structs)
    - [A.5.2 Step 2: Update Iterator State After `next()` And `remove()`](#a52-step-2-update-iterator-state-after-next-and-remove)
    - [A.5.3 Step 3: Reject Modification by Another Iterator](#a53-step-3-reject-modification-by-another-iterator)
  - [A.6 ThreadSanitizer](#a6-threadsanitizer)
    - [A.6.1 A simple Race Example](#a61-a-simple-race-example)
    - [A.6.2 Running A Program with TSan](#a62-running-a-program-with-tsan)
    - [A.6.3 Fixing A TSan Race](#a63-fixing-a-tsan-race)
    - [A.6.4 What TSan Does And Does Not Guarantee](#a64-what-tsan-does-and-does-not-guarantee)
  - [A.7 Exercise: Owala Resell Market](#a7-exercise-owala-resell-market)

---

### A.1 Critical Sections

A critical section is code that accesses shared data and must not run concurrently with conflicting code in another thread.

In a threaded program, shared data includes global variables, heap objects, static variables, and shared data structures. If multiple threads access the same data, and at least one thread writes to it, the access needs synchronization.

Example:

```c
int x = 0;
```

Multiple threads run:

```c
void *increase(void *arg) {
    (void)arg;
    x = x + 1;
    return NULL;
}
```

Although `x = x + 1` looks like one operation, it is really:

```text
read x
add 1
write x back
```

Two threads can interleave like this:

```text
x starts as 0

thread 1 reads x = 0
thread 2 reads x = 0
thread 1 writes x = 1
thread 2 writes x = 1
```

The expected result is:

```text
x = 2
```

but the actual result may be:

```text
x = 1
```

One update was lost. This is a race condition: the result depends on timing the program does not control.

#### A.1.1 Identifying the Critical Section

In this function:

```c
void *increase(void *arg) {
    (void)arg;
    x = x + 1;
    return NULL;
}
```

the critical section is:

```c
x = x + 1;
```

because it both reads and writes shared data.

The critical section must be protected so only one thread can execute it at a time.

#### A.1.2 Shared Data vs Local Data

Not all data needs a lock.

Local variables are usually safe:

```c
void *routine(void *arg) {
    int local = 0;
    local++;
    return NULL;
}
```

Each thread has its own stack, so each thread has its own copy of `local`.

Global data is shared:

```c
int x = 0;
```

If multiple threads modify `x`, it needs synchronization.

```text
thread-local data                  usually safe
shared read-only data              usually safe
shared data written by threads     needs synchronization
```

#### A.1.3 `volatile`

In C, `volatile` tells the compiler that a variable may change unexpectedly, outside the normal flow of the program.

A common mistake is thinking `volatile` makes threaded code safe:

```c
volatile int x = 0;
```

It does not. `volatile` does not make `x++` atomic, does not provide mutual exclusion, and does not prevent race conditions.

Another mistake is locking only the write:

```c
int temp = x;
temp++;

pthread_mutex_lock(&lock);
x = temp;
pthread_mutex_unlock(&lock);
```

This is still wrong because the read of `x` is also part of the critical section. The whole update must be protected:

```c
pthread_mutex_lock(&lock);
x = x + 1;
pthread_mutex_unlock(&lock);
```

---

### A.2 Mutexes

A **mutex** is a lock that gives **mutual exclusion** that only one thread can hold the mutex at a time.

The basic `pthread` pattern is:

```c
pthread_mutex_lock(&lock);
// Do something...
pthread_mutex_unlock(&lock);
```

A mutex prevents race conditions, such as the one shown in [A.1](#a1-critical-sections), by ensuring that one thread finishes the protected update before another thread starts the same update.

```c
#include <pthread.h>

static int x = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *increase(void *arg) {
    (void)arg;

    pthread_mutex_lock(&lock);
    x = x + 1;
    pthread_mutex_unlock(&lock);

    return NULL;
}
```

If two threads both call `increase()`, one possible order is:

```text
t1 locks lock
t1 updates x
t2 tries to lock lock and waits
t1 unlocks lock
t2 locks lock
t2 updates x
t2 unlocks lock
```

The exact order is not guaranteed. `t1` might go first, or `t2` might go first. The mutex only guarantees that they do not perform the protected update at the same time.

> [!WARNING]
> `volatile` does not replace a mutex.
> `volatile int x` does not make `x++` atomic, and it does not provide mutual exclusion. For shared updates, use a mutex or atomic operations.

A mutex works only when every relevant code path follows the same locking rule. The mutex is not automatically attached to a variable.

Good:

```c
pthread_mutex_lock(&lock);
x++;
pthread_mutex_unlock(&lock);
```

Still unsafe if another thread does this elsewhere:

```c
x++;
```

The rule should be:

```text
before accessing the shared data, lock the same mutex
after accessing the shared data, unlock that mutex
```

This also applies to reads. If one thread may write to `x` while another thread reads it, the read should usually be protected too:

```c
pthread_mutex_lock(&lock);
printf("%d\n", x);
pthread_mutex_unlock(&lock);
```

> [!NOTE]
> Protect the whole shared-data operation, not only the final assignment.
>
> ```c
> int temp = x;
> temp++;
>
> pthread_mutex_lock(&lock);
> x = temp;
> pthread_mutex_unlock(&lock);
> ```
>
> This is still unsafe because the read of `x` happened outside the lock. The read, modify, and write belong together.

When `pthread_mutex_lock()` returns successfully, the current thread owns the mutex. That thread should later release it with `pthread_mutex_unlock()`.

```c
pthread_mutex_lock(&lock);
/* this thread now owns lock */
pthread_mutex_unlock(&lock);
```

Only the thread that locked a mutex should unlock it. Unlocking a mutex from the wrong thread is not a valid synchronisation strategy and can lead to undefined behaviour depending on the mutex type.

Every successful lock should have a matching unlock on every path out of the function. This is especially important when there are early returns.

Bad:

```c
pthread_mutex_lock(&lock);

if (error) {
    return NULL;
}

pthread_mutex_unlock(&lock);
```

Better:

```c
pthread_mutex_lock(&lock);

if (error) {
    pthread_mutex_unlock(&lock);
    return NULL;
}

pthread_mutex_unlock(&lock);
```

Also keep the protected part as small as reasonably possible. The mutex should protect the shared data, not unrelated work.

Less useful:

```c
pthread_mutex_lock(&lock);

sleep(5);
x++;

pthread_mutex_unlock(&lock);
```

Better:

```c
sleep(5);

pthread_mutex_lock(&lock);
x++;
pthread_mutex_unlock(&lock);
```

If many threads spend most of their time waiting for one global mutex, the program may behave almost like a single-threaded program around that protected part. This is not a correctness bug, but it can reduce the benefit of using threads.

A complete small example:

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NTHREADS 5
#define NINCREMENTS 100000

static int x = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void *increase(void *arg) {
    (void)arg;

    for (int i = 0; i < NINCREMENTS; i++) {
        pthread_mutex_lock(&lock);
        x = x + 1;
        pthread_mutex_unlock(&lock);
    }

    return NULL;
}

int main(void) {
    pthread_t threads[NTHREADS];

    for (int i = 0; i < NTHREADS; i++) {
        int err = pthread_create(&threads[i], NULL, increase, NULL);

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

    printf("x = %d\n", x);

    pthread_mutex_destroy(&lock);
    return EXIT_SUCCESS;
}
```

Compile with:

```bash
gcc -std=c11 -Wall -Wextra -pedantic mutex_example.c -o mutex_example -pthread
```

Expected output:

```text
x = 500000
```

because:

```text
5 threads × 100000 increments each = 500000
```

Without the mutex, the final value may be smaller because some updates can be lost.

---

### A.3 Mutex Initialisation and Attributes

A `pthread_mutex_t` must be initialised before it is used. A mutex variable is not ready just because it exists.

This is wrong:

```c
pthread_mutex_t lock;

pthread_mutex_lock(&lock);
```

The mutex contains uninitialised data.

There are two common ways to initialise a mutex.

For a simple global, file-scope, or static mutex, use **static initialisation**:

```c
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
```

This gives the mutex a valid default initial state immediately.

For a mutex created at runtime, use **dynamic initialisation**:

```c
pthread_mutex_t lock;

pthread_mutex_init(&lock, NULL);

/* use lock */

pthread_mutex_destroy(&lock);
```

The second argument to `pthread_mutex_init()` is for mutex attributes. Passing `NULL` means:

```text
use the default mutex attributes
```

Dynamic initialisation is useful when the mutex is inside heap-allocated memory, inside a struct created at runtime, inside shared memory, or when you need non-default attributes.

> [!IMPORTANT]
> If a mutex is dynamically initialised with `pthread_mutex_init()`, clean it up with `pthread_mutex_destroy()` when it is no longer needed.
>
> Do not destroy a mutex while another thread might still lock it, unlock it, or wait on it. Usually, destroy the mutex only after the relevant threads have finished, for example after `pthread_join()`.

A mutex can also be initialised using an attribute object. A `pthread_mutexattr_t` is not the mutex itself. It is a temporary configuration object used when creating the mutex.

The lifecycle is:

```text
initialise attribute object
configure attributes
initialise mutex using those attributes
destroy attribute object
use mutex
destroy mutex
```

Example:

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    pthread_mutex_t lock;
    pthread_mutexattr_t attr;

    int err = pthread_mutexattr_init(&attr);
    if (err != 0) {
        fprintf(stderr, "pthread_mutexattr_init: %s\n", strerror(err));
        return EXIT_FAILURE;
    }

    err = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    if (err != 0) {
        fprintf(stderr, "pthread_mutexattr_settype: %s\n", strerror(err));
        pthread_mutexattr_destroy(&attr);
        return EXIT_FAILURE;
    }

    err = pthread_mutex_init(&lock, &attr);
    if (err != 0) {
        fprintf(stderr, "pthread_mutex_init: %s\n", strerror(err));
        pthread_mutexattr_destroy(&attr);
        return EXIT_FAILURE;
    }

    pthread_mutexattr_destroy(&attr);

    /*
     * Use the mutex here.
     */

    pthread_mutex_destroy(&lock);
    return EXIT_SUCCESS;
}
```

Destroying the attribute object does not destroy the mutex. After `pthread_mutex_init(&lock, &attr)` succeeds, the mutex has been initialised using those settings. The attribute object can then be cleaned up.

> [!NOTE]
> `PTHREAD_MUTEX_INITIALIZER` is for static initialisation. It is not a mutex type.
>
> This is wrong:
>
> ```c
> pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_INITIALIZER);
> ```
>
> For `pthread_mutexattr_settype()`, use a real mutex type such as `PTHREAD_MUTEX_NORMAL`, `PTHREAD_MUTEX_ERRORCHECK`, `PTHREAD_MUTEX_RECURSIVE`, or `PTHREAD_MUTEX_DEFAULT`.

The most useful mutex types to know are:

```text
PTHREAD_MUTEX_NORMAL       the usual basic mutex type
PTHREAD_MUTEX_ERRORCHECK   returns errors for some incorrect lock/unlock usage
PTHREAD_MUTEX_RECURSIVE    lets the same thread lock the same mutex multiple times
PTHREAD_MUTEX_DEFAULT      uses the implementation's default behaviour
```

A normal mutex is usually enough for beginner code:

```c
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
```

An error-checking mutex can be useful while debugging:

```c
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
```

It can report some incorrect usage, such as the wrong thread trying to unlock the mutex.

A recursive mutex allows the same thread to lock the same mutex more than once:

```c
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
```

For example:

```c
pthread_mutex_lock(&lock);
pthread_mutex_lock(&lock);

/* protected work */

pthread_mutex_unlock(&lock);
pthread_mutex_unlock(&lock);
```

The same thread must unlock it the same number of times. Recursive mutexes can be useful in some designs, but in beginner code they can also hide messy locking structure. Prefer a normal mutex unless there is a clear reason to use a recursive one.

`PTHREAD_MUTEX_DEFAULT` asks for the implementation's default behaviour. Do not rely on it having special recursive or error-checking behaviour. If you need those behaviours, request them explicitly.

> [!NOTE]
> GNU/Linux provides non-portable static initialisers such as:
>
> ```c
> PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
> PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP
> ```
>
> The `_NP` suffix means non-portable. For portable tutorial code, prefer dynamic initialisation with attributes:
>
> ```c
> pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
> pthread_mutex_init(&lock, &attr);
> ```

For normal COMP2017 examples, a default mutex is usually enough:

```c
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
```

or:

```c
pthread_mutex_t lock;

pthread_mutex_init(&lock, NULL);

/* use lock */

pthread_mutex_destroy(&lock);
```

More advanced attributes exist, such as `PTHREAD_PROCESS_SHARED` for mutexes stored in shared memory and used between processes, or `PTHREAD_PRIO_INHERIT` for scheduling-related behaviour. Those are useful in specific systems programming contexts, but they are not needed for the basic pthread mutex pattern.

---

### A.4 Exercise: Joey Doesn't Share Food

The plate is the shared resource. Only one person can hold the plate at a time, so the plate is represented by a mutex:

```c
pthread_mutex_t plate = PTHREAD_MUTEX_INITIALIZER;
```

The grapes are shared data:

```c
int grapes = 100;
```

So every time a thread checks or changes `grapes`, it should hold the plate mutex.

The behavior should be:

```text
Joey locks the plate
Joey keeps eating until grapes == 0
Joey unlocks the plate

Emma locks the plate
Emma eats at most one grape
Emma unlocks the plate
```

If Joey gets the plate first, Emma waits and gets no grapes. If Emma gets the plate first, she eats one grape, releases the plate, and then Joey eats the rest.

> [!IMPORTANT]
> Refer to [`friends.c`](./Codes/friends.c) for the solution to this exercise.
> You need to use the `-pthread` flag when compiling.

Creating Joey's thread first does not strictly guarantee Joey gets the mutex first.

This code:

```c
pthread_create(&joey_thread, NULL, joey, NULL);
pthread_create(&emma_thread, NULL, emma, NULL);
```

makes Joey eligible to run first, but the operating system decides the actual scheduling order.

So either of these can happen:

```text
Joey gets plate first -> Joey eats all grapes
Emma gets plate first -> Emma eats one grape, then Joey eats the rest
```

Both are valid threaded behavior.

---

### A.5 Exercise: Concurrent Linked List in C

The goal is to make the linked list detect unsafe concurrent modification.

The scaffold gives us a basic linked list and iterator. The iterator stores:

```c
struct ll* owner;
struct node* curr;
```

and the test creates one iterator in thread A, then another iterator in thread B that tries to remove while thread A is still iterating.

The rule we need is:

```text
If iterator A has already started iterating,
and iterator A has not finished,
then iterator B must not remove from the list.
```

If iterator B tries to remove, we print an error and terminate iterator B's thread with:

```c
pthread_exit(NULL);
```

Removing through the same iterator is still allowed.

We add bookkeeping to the linked list:

```c
pthread_mutex_t lock;
int active_iterators;
```

Then each iterator tracks whether it has started and whether it is currently counted as active:

```c
bool begun;
bool active;
```

An iterator is active when:

```text
it has called next() or remove() at least once
and it still has a next element
```

So the list can detect this situation:

```text
another iterator is still active
current iterator tries to remove
```

This is similar in spirit to Java's `ConcurrentModificationException`, but in C we manually print an error and call `pthread_exit()`.

> [!IMPORTANT]
> Refer to [`concurrent_ll.c`](./Codes/concurrent_ll.c) for the solution to this exercise.
> You need to use the `-pthread` flag when compiling.

#### A.5.1 Step 1: Extend the Structs

The list needs a mutex and an active iterator count.

The iterator needs extra state:

```c
bool begun;
bool active;
```

The mutex protects both:

```text
the linked list structure
the iterator bookkeeping
```

#### A.5.2 Step 2: Update Iterator State After `next()` And `remove()`

Calling `has_next()` should not start iteration.

But calling either of these should:

```c
ll_iterator_next(iterator);
ll_iterator_remove(iterator);
```

After the operation, the iterator is active only if:

```c
iterator->begun == true && iterator->curr != NULL
```

#### A.5.3 Step 3: Reject Modification by Another Iterator

Before `ll_iterator_remove()` deletes anything, it checks:

```text
Are there active iterators other than me?
```

If yes:

```text
print error
terminate this thread
```

If no, the remove is allowed.

This means thread A can remove from its own iterator, but thread B cannot remove while thread A is still mid-iteration.

> [!WARNING]
> A common mistake is only locking the actual `free()` operation. That is not enough. The traversal, pointer rewiring, iterator state update, and active iterator check must be protected together.
>
> Another common mistake is treating all removals as invalid. The exercise allows `iterator A removes through iterator A` and the invalid case is `iterator B removes while iterator A is active`.
>
> Also, `has_next()` should not mark the iterator as begun. The exercise says an iterator has begun once it has called `next` or `remove`.

---

### A.6 ThreadSanitizer

**ThreadSanitizer**, usually shortened to **TSan**, is a runtime debugging tool for finding **data races** in threaded programs.

A data race happens when:

```text
two or more threads access the same memory at the same time
at least one access is a write
there is no proper synchronization
```

For example:

```c
int counter = 0;

void *worker(void *arg) {
    counter++;
    return NULL;
}
```

If multiple threads run `worker()` at the same time, they may race on `counter`.

The line:

```c
counter++;
```

looks small, but it is not one indivisible operation. It is closer to:

```text
read counter
add 1
write counter back
```

So two threads can read the same old value and overwrite each other's updates.

TSan helps find this by instrumenting the program at compile time and checking memory accesses while the program runs. The Clang documentation describes ThreadSanitizer as a compiler instrumentation module plus runtime library for detecting data races.

TSan is especially useful when a threaded program behaves inconsistently:

```text
the result changes between runs
the bug disappears when printf is added
the program works with one thread but fails with many threads
a shared variable is read and written by multiple threads
```

These bugs are hard to find manually because they depend on thread scheduling. Two runs of the same program can behave differently.

#### A.6.1 A simple Race Example

```c
#include <pthread.h>
#include <stdio.h>

#define NTHREADS 4
#define NINCREMENTS 100000

static int counter = 0;

static void *worker(void *arg) {
    (void)arg;

    for (int i = 0; i < NINCREMENTS; i++) {
        counter++;
    }

    return NULL;
}

int main(void) {
    pthread_t threads[NTHREADS];

    for (int i = 0; i < NTHREADS; i++) {
        pthread_create(&threads[i], NULL, worker, NULL);
    }

    for (int i = 0; i < NTHREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("counter = %d\n", counter);
    return 0;
}
```

The expected final value is:

```text
counter = 400000
```

because:

```text
4 threads × 100000 increments each = 400000
```

But the actual result may be smaller because several threads can update `counter` at the same time.

The race is here:

```c
counter++;
```

All threads access the same shared variable, at least one access is a write, and there is no synchronization.

#### A.6.2 Running A Program with TSan

Compile with:

```bash
gcc -std=c11 -Wall -Wextra -g -O1 -fsanitize=thread race.c -o race -pthread
```

```text
-fsanitize=thread   enable ThreadSanitizer
-g                  include debugging information
-O1                 use light optimization; usually works well with sanitizer output
-pthread            compile and link pthread support
```

Then run the program normally:

```bash
./race
```

Clang's TSan documentation recommends compiling and linking with `-fsanitize=thread`, using `-O1` or higher for reasonable performance, and using `-g` for file names and line numbers.

With `-g`, TSan reports are much easier to read because they can include the source file and line number for each conflicting access.

A TSan report usually tells you:

```text
which memory location was involved
which thread read or wrote it
where the thread was created
which source code lines were involved
```

When reading the report, look for the two conflicting accesses. Usually, TSan will show one access from one thread and another access from another thread.

Then ask:

```text
Which shared variable is being accessed?
Is one of the accesses a write?
Are both accesses protected by the same mutex?
Should this variable be atomic?
Should this access happen only after pthread_join?
Should a condition variable be used?
```

> [!IMPORTANT]
> Do not ignore a TSan report just because the program "seems to work".
>
> A data race may not crash every time. It may work on one machine and fail on another. It may work with 2 threads and fail with 20. It may also appear or disappear when compiler optimization changes the timing.

#### A.6.3 Fixing A TSan Race

A correct version protects the shared update with a mutex:

```c
#include <pthread.h>
#include <stdio.h>

#define NTHREADS 4
#define NINCREMENTS 100000

static int counter = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void *worker(void *arg) {
    (void)arg;

    for (int i = 0; i < NINCREMENTS; i++) {
        pthread_mutex_lock(&lock);
        counter++;
        pthread_mutex_unlock(&lock);
    }

    return NULL;
}

int main(void) {
    pthread_t threads[NTHREADS];

    for (int i = 0; i < NTHREADS; i++) {
        pthread_create(&threads[i], NULL, worker, NULL);
    }

    for (int i = 0; i < NTHREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("counter = %d\n", counter);

    pthread_mutex_destroy(&lock);
    return 0;
}
```

Now every update to `counter` follows the same locking rule:

```c
pthread_mutex_lock(&lock);
counter++;
pthread_mutex_unlock(&lock);
```

TSan should no longer report a data race for `counter`.

> [!NOTE]
> Protect all relevant accesses, not just some of them.
>
> This is still unsafe if another thread reads `counter` elsewhere without the same mutex:
>
> ```c
> printf("%d\n", counter);
> ```
>
> If a shared variable is protected by a mutex, all relevant reads and writes should follow the same locking rule.

TSan is also useful for races involving other shared memory, such as:

```text
global variables
heap objects shared between threads
shared struct fields
shared linked-list pointers
shared flags that are not atomic
```

For example, this can also be a race:

```c
int ready = 0;

void *worker(void *arg) {
    ready = 1;
    return NULL;
}

int main(void) {
    pthread_t t;
    pthread_create(&t, NULL, worker, NULL);

    while (!ready) {
    }

    pthread_join(t, NULL);
}
```

One thread writes to `ready`, while another thread repeatedly reads `ready`. There is no mutex, atomic variable, or other synchronization.

A correct version should use a mutex, condition variable, or atomic variable, depending on the intended design.

#### A.6.4 What TSan Does And Does Not Guarantee

TSan only reports races that occur during the run it observes.

This means:

```text
No TSan warning does not prove the program is race-free.
```

It only means:

```text
TSan did not observe a race in that run.
```

So it is useful to test with:

```text
many threads
large loop counts
different inputs
repeated runs
stress tests
```

TSan is a debugging tool, not a proof of correctness.

TSan focuses on data races. It does not solve every concurrency problem. A program can have no TSan warnings and still have incorrect thread logic, missed notifications, starvation, or incorrect ordering between operations.

TSan also has practical limitations.

Clang's documentation says typical slowdown is around `5x-15x`, with typical memory overhead around `5x-10x`.

**TSan is mainly supported on 64-bit platforms.** The Clang documentation lists supported operating systems and architectures, and says support for 32-bit platforms is problematic and not planned.

For course environments, treat TSan as potentially unreliable on some lightweight virtualized environments such as WSL or Multipass if the course notes warn about that. TSan depends heavily on runtime instrumentation and virtual memory layout, so a specific VM, container, or subsystem can fail even if the operating system is generally supported.

TSan should also not be used in production builds. Clang describes it as a bug detection tool and says its runtime is not meant to be linked against production executables.

> [!IMPORTANT]
> Do not combine ThreadSanitizer with every other sanitizer.
>
> GCC documents that:
>
> ```text
> -fsanitize=thread
> ```
>
> cannot be combined with:
>
> ```text
> -fsanitize=address
> -fsanitize=leak
> ```
>
> in the same build.
>
> Make separate builds instead:
>
> ```bash
> # Thread race checking
> gcc -g -O1 -fsanitize=thread program.c -o program_tsan -pthread
>
> # Memory bug checking
> gcc -g -O1 -fsanitize=address program.c -o program_asan -pthread
> ```

A useful TSan workflow is:

```text
write pthread code
compile with -fsanitize=thread
run normal tests
run stress tests
read the TSan report
identify the shared variable
protect it with a mutex, atomic variable, or other synchronization
run again
```

For example:

```bash
gcc -std=c11 -Wall -Wextra -g -O1 -fsanitize=thread counter.c -o counter -pthread
./counter
```

The main idea is:

```text
Use TSan to find unsynchronized shared memory accesses.
Then fix the synchronization rule in the program.
```

TSan is very useful for pthread exercises involving shared counters, shared flags, shared structs, and shared data structures. But it should be treated as a debugging tool: it finds races that happen during the observed run, and it does not replace careful reasoning about thread behaviour.

---

### A.7 Exercise: Owala Resell Market

> [!IMPORTANT]
> Refer to [`owala_resellers.c`](./Codes/owala_resellers.c) for the solution to this exercise.
> You need to use the `-pthread` flag when compiling.

The order book has two shared linked lists:

```text
bids = BUY orders
asks = SELL orders
```

The writer thread reads input such as:

```text
BUY 30 20
SELL 10 18
```

and appends the order to the correct list.

The matcher thread wakes every 3 seconds, locks the order book, and tries to match orders.

A BUY can match SELL orders when:

```text
sell.price <= buy.price
```

A SELL can match BUY orders when:

```text
buy.price >= sell.price
```

No partial fills are allowed. So `BUY 30 20` can match `SELL 10 18` and `SELL 20 19`, because their quantities add exactly to `30`.

> [!NOTE]
> This solution uses one mutex for the whole order book. That is enough for this beginner exercise because both lists are part of the same shared market state.
>
> The matcher uses `sleep(3)`, as suggested by the exercise. Later, this could be improved with a condition variable.

**Example Usage:**

```bash
./owala_resellers
```

Then type:

```text
BUY 30 20
SELL 10 18
SELL 20 19
```

After about 3 seconds, the matcher prints:

```text
TRADE 30 @ 20
```
