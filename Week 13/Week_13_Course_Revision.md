## COMP2017 2026 S1 Revision Cheatsheets: Systems Programming

<table><tbody>
  <tr><td><b>Tutor</b></td><td>Hao Ren</td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au">hao.ren@sydney.edu.au</a></td></tr>
</tbody></table>

### Part NULL: Table of Contents and Compiling

- [COMP2017 2026 S1 Revision Cheatsheets: Systems Programming](#comp2017-2026-s1-revision-cheatsheets-systems-programming)
  - [Part NULL: Table of Contents and Compiling](#part-null-table-of-contents-and-compiling)
    - [0. How to Compile during Revision](#0-how-to-compile-during-revision)
  - [Part A: C Memory and Data Model](#part-a-c-memory-and-data-model)
    - [1. Pointers, Arrays, and `sizeof()`](#1-pointers-arrays-and-sizeof)
    - [2. C Strings](#2-c-strings)
    - [3. Stack vs Heap](#3-stack-vs-heap)
    - [4. `malloc()`, `calloc()`, `realloc()`, `free()`](#4-malloc-calloc-realloc-free)
    - [5. Linked Lists](#5-linked-lists)
    - [6. Structs, Padding, and Unions](#6-structs-padding-and-unions)
    - [7. Bitwise Operators and Flags](#7-bitwise-operators-and-flags)
    - [8. Function Pointers](#8-function-pointers)
  - [Part B: Files, File Descriptors, Processes, and IPC](#part-b-files-file-descriptors-processes-and-ipc)
    - [9. `FILE *` Stream I/O](#9-file--stream-io)
    - [10. File Descriptors: `open()`, `read()`, `write()`, `close()`](#10-file-descriptors-open-read-write-close)
    - [11. `fork()`, `wait()`, `waitpid()`, and `exec()`](#11-fork-wait-waitpid-and-exec)
    - [12. Signals, `struct sigaction` and `sigaction()`](#12-signals-struct-sigaction-and-sigaction)
    - [13. Pipes and `dup2()`](#13-pipes-and-dup2)
    - [14. File Descriptor Table Mental Model](#14-file-descriptor-table-mental-model)
    - [15. Non-Blocking I/O, `select()`, `poll()`, and `epoll()`](#15-non-blocking-io-select-poll-and-epoll)
    - [16. Shared Memory with `mmap()`](#16-shared-memory-with-mmap)
  - [Part C: Threads and synchronization](#part-c-threads-and-synchronization)
    - [17. `pthread_create()` and `pthread_join()`](#17-pthread_create-and-pthread_join)
    - [18. Race Conditions and Mutexes](#18-race-conditions-and-mutexes)
    - [19. Read-Write Locks](#19-read-write-locks)
    - [20. Deadlocks](#20-deadlocks)
    - [21. Semaphores](#21-semaphores)
    - [22. Condition Variables](#22-condition-variables)
    - [23. Recursion and Thread Overhead](#23-recursion-and-thread-overhead)

#### 0. How to Compile during Revision

Use different builds for different bug classes.

```bash
# Normal debug build
gcc -std=c11 -Wall -Wextra -Werror -g -o prog prog.c

# Memory bugs: invalid read/write, use-after-free, leaks
gcc -std=c11 -Wall -Wextra -g -fsanitize=address,undefined -o prog prog.c

# Thread race bugs: run separately from AddressSanitizer
gcc -std=c11 -Wall -Wextra -g -O1 -fsanitize=thread -pthread -o prog prog.c
```

> [!NOTE]
> **Exam Emphasis:** warnings are usually telling you something real. Do not ignore "incompatible pointer type", "implicit declaration", "may be uninitialized", or "format specifies type X but argument has type Y".

---

### Part A: C Memory and Data Model

#### 1. Pointers, Arrays, and `sizeof()`

Core idea: a pointer stores an address; an array owns a fixed block of elements. In most expressions, an array "decays" to a pointer to its first element. This is why students often misuse `sizeof`.

```c
int a[10];
int *p = a;

printf("%zu\n", sizeof a);     // 10 * sizeof(int)
printf("%zu\n", sizeof p);     // size of pointer, usually 8 on 64-bit
printf("%d\n", *(p + 3));      // same as a[3]
```

In a function parameter, `int a[]` is really `int *a`.

```c
void f(int a[]) {
    printf("%zu\n", sizeof a); // pointer size, NOT array size
}
```

Exam traps:

```c
int arr[5];
int n = sizeof(arr) / sizeof(arr[0]); // correct only in same scope

void bad(int arr[]) {
    int n = sizeof(arr) / sizeof(arr[0]); // wrong: arr is pointer here
}
```

> [!NOTE]
> **Exam Emphasis:** arrays know their size only where **the actual array object** exists. Pointers do not know how much memory they point to.

#### 2. C Strings

A C string is a `char` array ending with `'\0'`. `strlen(s)` counts characters before `'\0'`, not including the terminator. Forgetting `+1` is one of the most common bugs.

```c
char s[] = "cat";

printf("%zu\n", sizeof s); // 4: 'c', 'a', 't', '\0'
printf("%zu\n", strlen(s)); // 3
```

Safe string copy pattern:

```c
char *copy_string(const char *s) {
    size_t n = strlen(s) + 1;      // +1 for '\0'
    char *copy = malloc(n);
    if (copy == NULL) return NULL;

    memcpy(copy, s, n);            // copies the '\0' too
    return copy;
}
```

Fixed-size buffer pattern:

```c
char buf[64];

snprintf(buf, sizeof buf, "%s", "hello"); // guarantees '\0' if size > 0
```

Danger pattern:

```c
char buf[4];
strcpy(buf, "hello"); // wrong: needs 6 bytes including '\0'
```

`fgets` is usually safer than `scanf("%s", ...)` for reading strings because it takes a buffer size and reads at most `n - 1` bytes into the destination.

```c
char line[256];

if (fgets(line, sizeof line, stdin) != NULL) {
    line[strcspn(line, "\n")] = '\0'; // remove newline if present
}
```

> [!NOTE]
> **Exam Emphasis:** string bugs are usually **off-by-one**, **missing null terminator**, or **writing into memory you do not own**.

#### 3. Stack vs Heap

Stack memory is automatic and disappears when the function returns. Heap memory survives until `free`.

Bad:

```c
char *bad(void) {
    char buf[16] = "hello";
    return buf;              // wrong: returns pointer to dead stack memory
}
```

Good:

```c
char *good(void) {
    char *buf = malloc(16);
    if (buf == NULL) return NULL;

    strcpy(buf, "hello");
    return buf;              // caller must free
}
```

Ownership rule:

```c
char *s = good();
/* use s */
free(s);
s = NULL;
```

The person who receives heap memory must know who is responsible for freeing it. In exams, always ask: **Who owns this pointer? Who frees it? Can it outlive the function?**

#### 4. `malloc()`, `calloc()`, `realloc()`, `free()`

`malloc` gives uninitialized memory. `calloc` gives zero-initialized memory. `realloc` changes the size of an allocated object and preserves contents up to the smaller of the old and new sizes.

```c
int *a = malloc(n * sizeof *a);
if (a == NULL) {
    perror("malloc");
    exit(1);
}

free(a);
```

`calloc`:

```c
int *a = calloc(n, sizeof *a); // all elements initially 0
if (a == NULL) {
    perror("calloc");
    exit(1);
}
```

Correct `realloc` pattern:

```c
int *tmp = realloc(a, new_n * sizeof *a);
if (tmp == NULL) {
    // a is still valid here
    free(a);
    exit(1);
}
a = tmp;
```

Danger pattern:

```c
a = realloc(a, new_n * sizeof *a); // if realloc fails, original pointer is lost
```

Common memory bug vocabulary:

| Bug                | Meaning                                |
|--------------------|----------------------------------------|
| Memory leak        | Allocated memory is never freed        |
| Use-after-free     | Use pointer after `free(ptr)`          |
| Double free        | Call `free` twice on same allocation   |
| Dangling pointer   | Pointer still points to invalid memory |
| Buffer overflow    | Write past allocated boundary          |
| Uninitialized read | Read memory before assigning value     |

#### 5. Linked Lists

Linked lists test pointer ownership and pointer-to-pointer updates. The hardest part is deleting the head node correctly.

Basic node:

```c
typedef struct Node {
    int value;
    struct Node *next;
} Node;
```

Push front:

```c
void push_front(Node **head, int value) {
    Node *n = malloc(sizeof *n);
    if (n == NULL) abort();

    n->value = value;
    n->next = *head;
    *head = n;
}
```

Delete first matching value:

```c
#include <stdbool.h>

bool delete_value(Node **head, int value) {
    while (*head != NULL && (*head)->value != value) {
        head = &(*head)->next;
    }

    if (*head == NULL) return false;

    Node *victim = *head;
    *head = victim->next;
    free(victim);
    return true;
}
```

Free whole list:

```c
void free_list(Node *head) {
    while (head != NULL) {
        Node *next = head->next;
        free(head);
        head = next;
    }
}
```

> [!NOTE]
> **Exam Emphasis:** for linked-list deletion, update links **before** freeing the node. After `free(victim)`, do not read `victim->next`.

#### 6. Structs, Padding, and Unions

Structs group fields. Padding means `sizeof(struct)` may be larger than the sum of field sizes.

```c
typedef struct {
    char c;
    int x;
} Example;

printf("%zu\n", sizeof(Example)); // likely 8, not 5
```

Allocate structs like this:

```c
typedef struct {
    int id;
    char name[32];
} Student;

Student *s = malloc(sizeof *s);
if (s == NULL) exit(1);

s->id = 10;
snprintf(s->name, sizeof s->name, "%s", "Alice");

free(s);
```

Struct access:

```c
Student st;
st.id = 1;

Student *p = &st;
p->id = 2;       // same as (*p).id = 2
```

Union: all members share the same memory. Only one interpretation is valid at a time.

```c
union Value {
    int i;
    float f;
};

union Value v;
v.i = 42;
// Reading v.f here is not a meaningful "conversion".
```

> [!NOTE]
> **Exam Emphasis:** a struct stores all fields; a union overlays fields in the same storage.

#### 7. Bitwise Operators and Flags

Students often confuse logical operators with bitwise operators.

| Operator | Meaning     |
|----------|-------------|
| `&`      | bitwise AND |
| `\|`     | bitwise OR  |
| `^`      | bitwise XOR |
| `~`      | bitwise NOT |
| `<<`     | left shift  |
| `>>`     | right shift |

Flag pattern:

```c
#define FLAG_READ   (1u << 0)
#define FLAG_WRITE  (1u << 1)
#define FLAG_EXEC   (1u << 2)

unsigned flags = 0;

flags |= FLAG_READ;          // turn on
flags &= ~FLAG_WRITE;        // turn off
flags ^= FLAG_EXEC;          // toggle

if (flags & FLAG_READ) {
    puts("read enabled");
}
```

> [!NOTE]
> **Exam Emphasis:** use unsigned integers for bit operations when possible.

#### 8. Function Pointers

Function pointers are often tested through callbacks, dispatch tables. Comparator for `qsort`:

```c
#include <stdlib.h>

int cmp_int(const void *pa, const void *pb) {
    int a = *(const int *)pa;
    int b = *(const int *)pb;

    return (a > b) - (a < b);
}

int arr[] = {3, 1, 4, 2};
size_t n = sizeof arr / sizeof arr[0];

qsort(arr, n, sizeof arr[0], cmp_int);
```

Do not write this comparator:

```c
return *(int *)pa - *(int *)pb; // can overflow
```

Typedef pattern:

```c
typedef int (*operation_fn)(int, int);

int add(int a, int b) {
    return a + b;
}

operation_fn op = add;
printf("%d\n", op(2, 3));
```

> [!NOTE]
> **Exam Emphasis:** the callback receives `void *`, so you must cast to the correct pointer type before dereferencing.

---

### Part B: Files, File Descriptors, Processes, and IPC

#### 9. `FILE *` Stream I/O

`FILE *` is buffered standard-library I/O. Use it for formatted text processing.

Open, read lines, close:

```c
#include <stdio.h>

FILE *fp = fopen("input.txt", "r");
if (fp == NULL) {
    perror("fopen");
    return 1;
}

char line[256];
while (fgets(line, sizeof line, fp) != NULL) {
    printf("%s", line);
}

if (fclose(fp) == EOF) {
    perror("fclose");
}
```

Correct `fscanf` pattern:

```c
int id;
char name[64];

while (fscanf(fp, "%d %63s", &id, name) == 2) {
    printf("%d %s\n", id, name);
}
```

Bad pattern:

```c
while (!feof(fp)) {
    fscanf(fp, "%d", &x); // wrong: feof becomes true only after read fails
}
```

> [!NOTE]
> **Exam Emphasis:** always check how many items `fscanf` successfully read.

#### 10. File Descriptors: `open()`, `read()`, `write()`, `close()`

File descriptors are small integers managed by the kernel. `STDIN_FILENO` is 0, `STDOUT_FILENO` is 1, and `STDERR_FILENO` is 2.

```c
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int fd = open("input.txt", O_RDONLY);
if (fd == -1) {
    perror("open");
    return 1;
}

char buf[4096];
ssize_t n;

while ((n = read(fd, buf, sizeof buf)) > 0) {
    write(STDOUT_FILENO, buf, (size_t)n);
}

if (n == -1) {
    perror("read");
}

close(fd);
```

`read` attempts to read up to the requested number of bytes; it may read fewer. `write` may also write fewer bytes, especially for pipes, sockets, or interrupted calls.

Robust `write_all` helper:

```c
#include <errno.h>
#include <unistd.h>

int write_all(int fd, const void *buf, size_t len) {
    const char *p = buf;

    while (len > 0) {
        ssize_t n = write(fd, p, len);

        if (n == -1) {
            if (errno == EINTR) continue;
            return -1;
        }

        if (n == 0) return -1;

        p += n;
        len -= (size_t)n;
    }

    return 0;
}
```

> [!NOTE]
> **Exam Emphasis:** `read` and `write` work with raw bytes; `fgets` and `fprintf` work with buffered streams.

#### 11. `fork()`, `wait()`, `waitpid()`, and `exec()`

`fork()` creates a child process; the child is a copy of the parent except for details such as PID and return value. `exec` replaces the current process image with a new program.

Standard pattern:

```c
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

pid_t pid = fork();

if (pid == -1) {
    perror("fork");
    exit(1);
}

if (pid == 0) {
    // Child
    execlp("ls", "ls", "-l", (char *)NULL);

    // Only reached if exec fails
    perror("execlp");
    _exit(127);
}

// Parent
int status;
if (waitpid(pid, &status, 0) == -1) {
    perror("waitpid");
    exit(1);
}

if (WIFEXITED(status)) {
    printf("child exited with %d\n", WEXITSTATUS(status));
} else if (WIFSIGNALED(status)) {
    printf("child killed by signal %d\n", WTERMSIG(status));
}
```

Return values of `fork`:

```c
pid < 0   // error
pid == 0  // child process
pid > 0   // parent process; pid is child's PID
```

> [!NOTE]
> **Exam Emphasis:**
>
> | Question                                      | Answer                                       |
> |-----------------------------------------------|----------------------------------------------|
> | Does child share local variables with parent? | No, logically separate memory after `fork`   |
> | Does child inherit file descriptors?          | Yes                                          |
> | Does `exec` create a new process?             | No, it replaces the current process image    |
> | Does `exec` return on success?                | No                                           |
> | Why `_exit` in child after failed `exec`?     | Avoid flushing parent's buffered stdio again |

Common process-count question:

```c
fork();
fork();
fork();
```

This creates `2^3 = 8` processes total, assuming all forks succeed.

#### 12. Signals, `struct sigaction` and `sigaction()`

Signals are asynchronous notifications. Students often write unsafe signal handlers.

Safe pattern: set a flag.

```c
#include <signal.h>
#include <string.h>
#include <unistd.h>

static volatile sig_atomic_t stop = 0;

static void on_sigint(int sig) {
    (void)sig;
    stop = 1;
}

int main(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);

    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGINT, &sa, NULL);

    while (!stop) {
        // normal program work
    }

    write(STDOUT_FILENO, "stopping\n", 9);
}
```

Inside a signal handler, do not call unsafe functions such as `printf`, `malloc`, `free`, or `pthread_mutex_lock`. Prefer setting a `sig_atomic_t` flag or using `write` carefully.

> [!NOTE]
> **Exam Emphasis:** `SIGKILL` and `SIGSTOP` cannot be caught, blocked, or ignored.

#### 13. Pipes and `dup2()`

`pipe()` creates two file descriptors: `p[0]` for reading and `p[1]` for writing.

Single child writes to parent:

```c
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int p[2];

if (pipe(p) == -1) {
    perror("pipe");
    exit(1);
}

pid_t pid = fork();

if (pid == 0) {
    close(p[0]);                         // child does not read
    dup2(p[1], STDOUT_FILENO);           // stdout -> pipe write end
    close(p[1]);

    execlp("ls", "ls", (char *)NULL);
    perror("execlp");
    _exit(127);
}

close(p[1]);                             // parent does not write

char buf[4096];
ssize_t n;
while ((n = read(p[0], buf, sizeof buf)) > 0) {
    write(STDOUT_FILENO, buf, (size_t)n);
}

close(p[0]);
waitpid(pid, NULL, 0);
```

Two-command pipeline: `ls | wc -l`

```c
int p[2];
pipe(p);

if (fork() == 0) {
    dup2(p[1], STDOUT_FILENO);
    close(p[0]);
    close(p[1]);
    execlp("ls", "ls", (char *)NULL);
    _exit(127);
}

if (fork() == 0) {
    dup2(p[0], STDIN_FILENO);
    close(p[1]);
    close(p[0]);
    execlp("wc", "wc", "-l", (char *)NULL);
    _exit(127);
}

close(p[0]);
close(p[1]);

while (wait(NULL) > 0) {
}
```

> [!NOTE]
> **Exam Emphasis:** close unused pipe ends. If a process keeps a write end open, the reader may block forever waiting for EOF.

#### 14. File Descriptor Table Mental Model

After `fork`, both parent and child have file descriptors referring to the same underlying open file descriptions.

Draw this in exams:

```text
Parent fd table:
0 -> stdin
1 -> stdout
2 -> stderr
3 -> pipe read end
4 -> pipe write end

Child after fork:
0 -> stdin
1 -> stdout
2 -> stderr
3 -> same pipe read end
4 -> same pipe write end
```

After:

```c
dup2(p[1], STDOUT_FILENO);
```

the child has:

```text
1 -> pipe write end
```

So `printf`, `puts`, and `write(STDOUT_FILENO, ...)` now go into the pipe.

> [!NOTE]
> **Exam Emphasis:** `dup2(oldfd, newfd)` makes `newfd` refer to the same open file description as `oldfd`.

#### 15. Non-Blocking I/O, `select()`, `poll()`, and `epoll()`

Non-blocking I/O means a call may fail with `EAGAIN` or `EWOULDBLOCK` instead of waiting.

```c
#include <fcntl.h>
#include <unistd.h>

int flags = fcntl(fd, F_GETFL, 0);
fcntl(fd, F_SETFL, flags | O_NONBLOCK);
```

Simple `poll` pattern:

```c
#include <poll.h>
#include <unistd.h>
#include <stdio.h>

struct pollfd fds[1];

fds[0].fd = STDIN_FILENO;
fds[0].events = POLLIN;

int rc = poll(fds, 1, 5000); // wait up to 5000 ms

if (rc == -1) {
    perror("poll");
} else if (rc == 0) {
    puts("timeout");
} else if (fds[0].revents & POLLIN) {
    char buf[128];
    ssize_t n = read(STDIN_FILENO, buf, sizeof buf);
    if (n > 0) {
        write(STDOUT_FILENO, buf, (size_t)n);
    }
}
```

> [!NOTE]
> **Exam Emphasis:** `select`, `poll`, and `epoll` do not magically read data. They tell you which descriptors are ready; you still call `read` or `write`.

#### 16. Shared Memory with `mmap()`

Shared memory lets processes access the same memory region. But shared memory does not automatically make operations safe.

```c
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int *counter = mmap(NULL, sizeof *counter,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS,
                    -1, 0);

if (counter == MAP_FAILED) {
    perror("mmap");
    exit(1);
}

*counter = 0;

pid_t pid = fork();

if (pid == 0) {
    (*counter)++;
    _exit(0);
}

waitpid(pid, NULL, 0);
printf("%d\n", *counter);

munmap(counter, sizeof *counter);
```

> [!NOTE]
> **Exam Emphasis:** if multiple processes update shared memory concurrently, you still need synchronization, such as semaphores.

---

### Part C: Threads and synchronization

#### 17. `pthread_create()` and `pthread_join()`

`pthread_create` creates a new thread within the same process. Threads share process memory, including global variables and heap memory.

Compile with `-pthread`.

```c
#include <pthread.h>
#include <stdio.h>

void *worker(void *arg) {
    int id = *(int *)arg;
    printf("thread %d\n", id);
    return NULL;
}

int main(void) {
    enum { N = 4 };

    pthread_t tids[N];
    int ids[N];

    for (int i = 0; i < N; i++) {
        ids[i] = i;
        pthread_create(&tids[i], NULL, worker, &ids[i]);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(tids[i], NULL);
    }
}
```

Bad pattern:

```c
for (int i = 0; i < N; i++) {
    pthread_create(&tids[i], NULL, worker, &i); // wrong: all threads share &i
}
```

> [!NOTE]
> **Exam Emphasis:** thread arguments must remain valid until the thread has read them.

#### 18. Race Conditions and Mutexes

A race condition occurs when multiple threads access shared state concurrently and at least one access is a write.

Wrong:

```c
int counter = 0;

void *worker(void *arg) {
    for (int i = 0; i < 100000; i++) {
        counter++; // race
    }
    return NULL;
}
```

Correct:

```c
#include <pthread.h>

int counter = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *worker(void *arg) {
    for (int i = 0; i < 100000; i++) {
        pthread_mutex_lock(&lock);
        counter++;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}
```

`pthread_mutex_lock` locks a mutex; `pthread_mutex_unlock` unlocks it.

> [!NOTE]
> **Exam Emphasis:** protect the shared data, not the line of code. Ask: **What invariant must not be observed halfway updated?**

#### 19. Read-Write Locks

Use read-write locks when many threads read shared data, but writers need exclusive access.

```c
#include <pthread.h>

pthread_rwlock_t rw = PTHREAD_RWLOCK_INITIALIZER;
int shared_value = 0;

void read_value(void) {
    pthread_rwlock_rdlock(&rw);
    int local = shared_value;
    pthread_rwlock_unlock(&rw);

    printf("%d\n", local);
}

void write_value(int x) {
    pthread_rwlock_wrlock(&rw);
    shared_value = x;
    pthread_rwlock_unlock(&rw);
}
```

> [!NOTE]
> **Exam Emphasis:** multiple readers can hold the lock together, but a writer must be alone.

#### 20. Deadlocks

Deadlock usually requires four conditions: mutual exclusion, hold-and-wait, no preemption, and circular wait.

Classic mistake:

```c
// Thread 1
pthread_mutex_lock(&a);
pthread_mutex_lock(&b);

// Thread 2
pthread_mutex_lock(&b);
pthread_mutex_lock(&a);
```

Fix with a global lock order:

```c
typedef struct {
    pthread_mutex_t mutex;
    int rank;
} RankedLock;

void lock_two(RankedLock *x, RankedLock *y) {
    if (x->rank > y->rank) {
        RankedLock *tmp = x;
        x = y;
        y = tmp;
    }

    pthread_mutex_lock(&x->mutex);
    pthread_mutex_lock(&y->mutex);
}

void unlock_two(RankedLock *x, RankedLock *y) {
    pthread_mutex_unlock(&y->mutex);
    pthread_mutex_unlock(&x->mutex);
}
```

> [!NOTE]
> **Exam Emphasis:** when diagnosing deadlock, draw a wait-for graph: thread A holds lock 1 and waits for lock 2; thread B holds lock 2 and waits for lock 1.

#### 21. Semaphores

A semaphore is a counter used for coordination. `sem_wait` locks/decrements the semaphore; if the value is zero, the caller waits. `sem_post` unlocks/increments the semaphore.

Basic use:

```c
#include <semaphore.h>

sem_t sem;

sem_init(&sem, 0, 1); // 1 means initially available

sem_wait(&sem);
/* critical or limited section */
sem_post(&sem);

sem_destroy(&sem);
```

Producer-consumer pattern:

```c
#define CAP 8

sem_t empty_slots;
sem_t full_slots;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

void init(void) {
    sem_init(&empty_slots, 0, CAP);
    sem_init(&full_slots, 0, 0);
}

void produce(int item) {
    sem_wait(&empty_slots);

    pthread_mutex_lock(&m);
    // insert item into buffer
    pthread_mutex_unlock(&m);

    sem_post(&full_slots);
}

int consume(void) {
    sem_wait(&full_slots);

    pthread_mutex_lock(&m);
    // remove item from buffer
    int item = 0;
    pthread_mutex_unlock(&m);

    sem_post(&empty_slots);
    return item;
}
```

> [!NOTE]
> **Exam Emphasis:** semaphores are good for counting resources or enforcing ordering. Mutexes are better for ownership of a critical section.

#### 22. Condition Variables

Condition variables are for waiting until a predicate becomes true. They must be used with a mutex. `pthread_cond_wait` is called with the mutex locked, and it blocks on the condition variable while releasing the mutex during the wait.

Correct pattern:

```c
#include <pthread.h>

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

int ready = 0;

void *consumer(void *arg) {
    pthread_mutex_lock(&m);

    while (!ready) {
        pthread_cond_wait(&cv, &m);
    }

    // ready is true here
    pthread_mutex_unlock(&m);
    return NULL;
}

void producer(void) {
    pthread_mutex_lock(&m);
    ready = 1;
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&m);
}
```

Important: use `while`, not `if`.

Bad:

```c
if (!ready) {
    pthread_cond_wait(&cv, &m);
}
```

Good:

```c
while (!ready) {
    pthread_cond_wait(&cv, &m);
}
```

> [!NOTE]
> **Exam Emphasis:** condition variables do not store "signals" like message queues. They are tied to a predicate protected by a mutex.

#### 23. Recursion and Thread Overhead

Recursion needs a base case and consumes stack space.

```c
int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}
```

Equivalent iterative form:

```c
int factorial_iter(int n) {
    int ans = 1;

    for (int i = 2; i <= n; i++) {
        ans *= i;
    }

    return ans;
}
```

Bad idea: create a new thread for every recursive Fibonacci call.

```c
// Conceptually bad: too many threads
fib(n) = fib(n - 1) + fib(n - 2);
```

> [!NOTE]
> **Exam Emphasis:** recursion may be elegant, but it is not automatically efficient. Thread creation has overhead; parallelism helps only when the work is large enough and synchronization is controlled.
