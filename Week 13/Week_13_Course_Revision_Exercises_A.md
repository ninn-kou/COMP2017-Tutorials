## COMP2017 2026 S1 Course Revision Exercises: Solutions

<table><tbody>
  <tr><td><b>Tutor</b></td><td>Hao Ren</td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au">hao.ren@sydney.edu.au</a></td></tr>
</tbody></table>

- [COMP2017 2026 S1 Course Revision Exercises: Solutions](#comp2017-2026-s1-course-revision-exercises-solutions)
  - [Exercise 1 Solution: Process Pool with Pipes, `select()`, Signals, and File Descriptors](#exercise-1-solution-process-pool-with-pipes-select-signals-and-file-descriptors)
  - [Exercise 2 Solution: Shared-Memory Parallel Reduction with Processes and Semaphores](#exercise-2-solution-shared-memory-parallel-reduction-with-processes-and-semaphores)
  - [Exercise 3 Solution: Thread Pool with Condition Variables, Task Queue, and Shutdown](#exercise-3-solution-thread-pool-with-condition-variables-task-queue-and-shutdown)
  - [Exercise 4 Solution: Parallel Divide-and-Conquer with Reductions and Load Balancing](#exercise-4-solution-parallel-divide-and-conquer-with-reductions-and-load-balancing)
  - [Exercise 5 Solution: Signal-Controlled `exec()` Runner with Timeout](#exercise-5-solution-signal-controlled-exec-runner-with-timeout)
  - [Exercise 6 Solution: Shared Memory, `fork()`, Race Condition, and Virtual Memory Reasoning](#exercise-6-solution-shared-memory-fork-race-condition-and-virtual-memory-reasoning)
  - [Exercise 7 Solution: Cache, Branch Prediction, and Parallel Matrix Reduction](#exercise-7-solution-cache-branch-prediction-and-parallel-matrix-reduction)
  - [Exercise 8 Solution: Security, System Calls, and File-Copy Robustness](#exercise-8-solution-security-system-calls-and-file-copy-robustness)
  - [Exercise 9 Solution: Compiler/Linker, Static Symbols, Shared Libraries, and Process Memory](#exercise-9-solution-compilerlinker-static-symbols-shared-libraries-and-process-memory)
  - [Exercise 10 Solution: Cross-Topic Debugging: Race, Deadlock, and Memory Lifetime](#exercise-10-solution-cross-topic-debugging-race-deadlock-and-memory-lifetime)

---

### Exercise 1 Solution: Process Pool with Pipes, `select()`, Signals, and File Descriptors

This problem tests:

```text
fork()
pipe()
read()/write()
select()
SIGINT safety
file descriptor ownership
waitpid()
partial read/write awareness
```

We use two pipes per worker:

```text
parent_to_child[i][1]  parent writes jobs
parent_to_child[i][0]  child reads jobs

child_to_parent[i][1]  child writes results
child_to_parent[i][0]  parent reads results
```

Global state:

```c
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define MAX_WORKERS 64

static int to_child[MAX_WORKERS][2];
static int from_child[MAX_WORKERS][2];
static pid_t workers[MAX_WORKERS];
static int nworkers;

static volatile sig_atomic_t got_sigint = 0;
```

Signal handler:

```c
static void on_sigint(int sig) {
    (void)sig;
    got_sigint = 1;
}
```

Use robust integer read/write helpers. Even though `sizeof(int)` is small, exam solutions should show awareness that `read()` and `write()` are byte-stream operations.

```c
static int write_full(int fd, const void *buf, size_t n) {
    const char *p = buf;

    while (n > 0) {
        ssize_t w = write(fd, p, n);

        if (w == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }

        if (w == 0) {
            return -1;
        }

        p += w;
        n -= (size_t)w;
    }

    return 0;
}

static int read_full(int fd, void *buf, size_t n) {
    char *p = buf;

    while (n > 0) {
        ssize_t r = read(fd, p, n);

        if (r == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }

        if (r == 0) {
            return -1;
        }

        p += r;
        n -= (size_t)r;
    }

    return 0;
}
```

Worker loop:

```c
static void worker_loop(int id) {
    /*
     * Child only uses:
     *     to_child[id][0]    for reading jobs
     *     from_child[id][1]  for writing results
     */

    int job;

    for (;;) {
        if (read_full(to_child[id][0], &job, sizeof job) == -1) {
            _exit(1);
        }

        if (job == -1) {
            _exit(0);
        }

        int result = job * job;

        if (write_full(from_child[id][1], &result, sizeof result) == -1) {
            _exit(1);
        }
    }
}
```

Create workers:

```c
void create_workers(int N) {
    nworkers = N;

    for (int i = 0; i < N; i++) {
        if (pipe(to_child[i]) == -1) {
            perror("pipe");
            exit(1);
        }

        if (pipe(from_child[i]) == -1) {
            perror("pipe");
            exit(1);
        }

        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(1);
        }

        if (pid == 0) {
            /*
             * Child process.
             *
             * Close pipe ends that this child does not use.
             */
            close(to_child[i][1]);      // child does not write jobs
            close(from_child[i][0]);    // child does not read results

            /*
             * Close pipes belonging to previously created workers.
             * Otherwise children accidentally keep unrelated pipe ends open.
             */
            for (int j = 0; j < i; j++) {
                close(to_child[j][0]);
                close(to_child[j][1]);
                close(from_child[j][0]);
                close(from_child[j][1]);
            }

            worker_loop(i);
            _exit(0);
        }

        /*
         * Parent process.
         */
        workers[i] = pid;

        close(to_child[i][0]);      // parent does not read jobs
        close(from_child[i][1]);    // parent does not write results
    }
}
```

Send one job:

```c
void send_job(int worker, int job) {
    write_full(to_child[worker][1], &job, sizeof job);
}
```

Parent loop:

```c
void parent_loop(int M) {
    int next_job = 1;
    int results_received = 0;

    /*
     * Initially give one job to each worker, if possible.
     */
    for (int i = 0; i < nworkers && next_job <= M; i++) {
        send_job(i, next_job);
        next_job++;
    }

    while (results_received < M && !got_sigint) {
        fd_set rfds;
        FD_ZERO(&rfds);

        int maxfd = -1;

        for (int i = 0; i < nworkers; i++) {
            FD_SET(from_child[i][0], &rfds);

            if (from_child[i][0] > maxfd) {
                maxfd = from_child[i][0];
            }
        }

        int rc = select(maxfd + 1, &rfds, NULL, NULL, NULL);

        if (rc == -1) {
            if (errno == EINTR) {
                continue;
            }

            perror("select");
            break;
        }

        for (int i = 0; i < nworkers; i++) {
            if (FD_ISSET(from_child[i][0], &rfds)) {
                int result;

                if (read_full(from_child[i][0], &result, sizeof result) == 0) {
                    printf("result = %d\n", result);
                    results_received++;

                    /*
                     * Send another job to the worker that just became free.
                     */
                    if (next_job <= M) {
                        send_job(i, next_job);
                        next_job++;
                    }
                }
            }
        }
    }

    cleanup();
}
```

Cleanup:

```c
void cleanup(void) {
    /*
     * Ask children to stop.
     */
    for (int i = 0; i < nworkers; i++) {
        int stop = -1;
        write_full(to_child[i][1], &stop, sizeof stop);
    }

    /*
     * Close parent pipe ends.
     */
    for (int i = 0; i < nworkers; i++) {
        close(to_child[i][1]);
        close(from_child[i][0]);
    }

    /*
     * Reap children.
     */
    for (int i = 0; i < nworkers; i++) {
        waitpid(workers[i], NULL, 0);
    }
}
```

Install the signal handler:

```c
int main(void) {
    struct sigaction sa;

    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);

    create_workers(4);
    parent_loop(100);

    return 0;
}
```

Most common mistakes:

```text
Keeping unused pipe ends open.
Forgetting that select() only tells you readiness; it does not read data.
Using printf() inside a signal handler.
Failing to reap children.
Sending all jobs to one worker instead of load-balancing.
Assuming one write() always equals one read().
```

The key file-descriptor diagram for each worker is:

```text
Parent:
    to_child[i][1]      open
    from_child[i][0]    open

Child i:
    to_child[i][0]      open
    from_child[i][1]    open
```

Everything else should be closed.

---

### Exercise 2 Solution: Shared-Memory Parallel Reduction with Processes and Semaphores

This exercise combines:

```text
fork()
virtual memory
mmap()
shared memory
semaphores
parallel reductions
load balancing
cache effects
```

Shared state:

```c
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>

struct shared_state {
    sem_t lock;
    long long global_sum;
};
```

Worker range calculation:

```c
static void get_range(int worker, int P, int N, int *start, int *end) {
    int base = N / P;
    int rem = N % P;

    if (worker < rem) {
        *start = worker * (base + 1);
        *end = *start + base + 1;
    } else {
        *start = rem * (base + 1) + (worker - rem) * base;
        *end = *start + base;
    }
}
```

This gives balanced chunks even when `N` is not divisible by `P`.

Main solution:

```c
void parallel_sum_processes(int *data, int N, int P) {
    struct shared_state *shared = mmap(NULL,
                                       sizeof *shared,
                                       PROT_READ | PROT_WRITE,
                                       MAP_SHARED | MAP_ANONYMOUS,
                                       -1,
                                       0);

    if (shared == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    shared->global_sum = 0;

    /*
     * pshared = 1 means the semaphore is shared between processes.
     */
    if (sem_init(&shared->lock, 1, 1) == -1) {
        perror("sem_init");
        exit(1);
    }

    for (int w = 0; w < P; w++) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(1);
        }

        if (pid == 0) {
            int start;
            int end;

            get_range(w, P, N, &start, &end);

            /*
             * Good reduction pattern:
             * compute local sum without lock,
             * then lock once to update global sum.
             */
            long long local = 0;

            for (int i = start; i < end; i++) {
                local += data[i];
            }

            sem_wait(&shared->lock);
            shared->global_sum += local;
            sem_post(&shared->lock);

            _exit(0);
        }
    }

    for (int w = 0; w < P; w++) {
        wait(NULL);
    }

    printf("sum = %lld\n", shared->global_sum);

    sem_destroy(&shared->lock);
    munmap(shared, sizeof *shared);
}
```

Why `mmap()` is needed:

```text
After fork(), parent and child processes have separate virtual address spaces.
Ordinary heap/global variables are copied logically, usually by copy-on-write.
If a child modifies a normal variable, the parent does not see that modification.
MAP_SHARED mmap memory creates a region visible to all related processes.
```

Why `sem_init(..., 1, ...)` is needed:

```text
The second argument pshared must be nonzero when the semaphore is shared between processes.
If pshared is 0, the semaphore is only intended for threads within one process.
```

Why locking per element is bad:

Bad version:

```c
for (int i = start; i < end; i++) {
    sem_wait(&shared->lock);
    shared->global_sum += data[i];
    sem_post(&shared->lock);
}
```

This destroys parallelism because all workers serialize on one semaphore for every element.

Good version:

```c
long long local = 0;

for (int i = start; i < end; i++) {
    local += data[i];
}

sem_wait(&shared->lock);
shared->global_sum += local;
sem_post(&shared->lock);
```

This reduces synchronization from:

```text
O(N) lock operations
```

to:

```text
O(P) lock operations
```

False sharing issue:

If we instead used:

```c
long long partial[P];
```

in shared memory, each process could write:

```c
partial[w] = local;
```

This avoids a semaphore. However, adjacent `partial[w]` values may live on the same cache line. Different processes writing different elements on the same cache line can still cause cache-coherence traffic.

A padded version is better:

```c
struct padded_sum {
    long long value;
    char padding[64 - sizeof(long long)];
};
```

Then:

```c
struct padded_sum partial[P];
```

This gives each worker's partial sum its own cache-line-sized slot.

Most common mistakes:

```text
Using ordinary malloc instead of mmap for shared state.
Using sem_init(&sem, 0, 1) for process-shared synchronization.
Locking once per array element.
Forgetting wait() and reading the sum before children finish.
Assuming forked children share normal global variables.
Ignoring load imbalance when N is not divisible by P.
```

---

### Exercise 3 Solution: Thread Pool with Condition Variables, Task Queue, and Shutdown

This problem combines:

```text
threads
mutexes
condition variables
bounded buffers
parallel algorithms
load balancing
lifetime management
```

Data structures:

```c
#include <pthread.h>
#include <stdlib.h>

#define CAP 64
#define MAX_WORKERS 32

typedef void (*task_fn)(void *);

struct task {
    task_fn fn;
    void *arg;
};

struct pool {
    pthread_t workers[MAX_WORKERS];
    int W;

    struct task q[CAP];
    int head;
    int tail;
    int count;

    int shutdown;

    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
};
```

Worker loop:

```c
static void *worker_main(void *arg) {
    struct pool *p = arg;

    for (;;) {
        pthread_mutex_lock(&p->lock);

        while (p->count == 0 && !p->shutdown) {
            pthread_cond_wait(&p->not_empty, &p->lock);
        }

        if (p->count == 0 && p->shutdown) {
            pthread_mutex_unlock(&p->lock);
            return NULL;
        }

        struct task t = p->q[p->head];
        p->head = (p->head + 1) % CAP;
        p->count--;

        pthread_cond_signal(&p->not_full);

        pthread_mutex_unlock(&p->lock);

        /*
         * Run the task outside the lock.
         */
        t.fn(t.arg);
    }
}
```

Initialization:

```c
void pool_init(struct pool *p, int W) {
    p->W = W;
    p->head = 0;
    p->tail = 0;
    p->count = 0;
    p->shutdown = 0;

    pthread_mutex_init(&p->lock, NULL);
    pthread_cond_init(&p->not_empty, NULL);
    pthread_cond_init(&p->not_full, NULL);

    for (int i = 0; i < W; i++) {
        pthread_create(&p->workers[i], NULL, worker_main, p);
    }
}
```

Submit task:

```c
void pool_submit(struct pool *p, task_fn fn, void *arg) {
    pthread_mutex_lock(&p->lock);

    while (p->count == CAP && !p->shutdown) {
        pthread_cond_wait(&p->not_full, &p->lock);
    }

    if (p->shutdown) {
        pthread_mutex_unlock(&p->lock);
        return;
    }

    p->q[p->tail].fn = fn;
    p->q[p->tail].arg = arg;
    p->tail = (p->tail + 1) % CAP;
    p->count++;

    pthread_cond_signal(&p->not_empty);

    pthread_mutex_unlock(&p->lock);
}
```

Shutdown:

```c
void pool_shutdown(struct pool *p) {
    pthread_mutex_lock(&p->lock);

    p->shutdown = 1;

    pthread_cond_broadcast(&p->not_empty);
    pthread_cond_broadcast(&p->not_full);

    pthread_mutex_unlock(&p->lock);

    for (int i = 0; i < p->W; i++) {
        pthread_join(p->workers[i], NULL);
    }

    pthread_cond_destroy(&p->not_empty);
    pthread_cond_destroy(&p->not_full);
    pthread_mutex_destroy(&p->lock);
}
```

Important reasoning:

```text
Workers wait while the queue is empty.
Submitters wait while the queue is full.
The condition variable is always used with a predicate:
    count == 0
    count == CAP
    shutdown
```

Always use `while`, not `if`:

```c
while (p->count == 0 && !p->shutdown) {
    pthread_cond_wait(&p->not_empty, &p->lock);
}
```

Do not run tasks while holding the mutex. This would serialize the whole pool.

Bad:

```c
pthread_mutex_lock(&p->lock);
t.fn(t.arg);
pthread_mutex_unlock(&p->lock);
```

Good:

```c
remove task under lock
unlock
run task
```

Most common mistakes:

```text
Using if instead of while around pthread_cond_wait().
Forgetting to broadcast during shutdown.
Running tasks while holding the queue mutex.
Destroying the mutex before joining workers.
Not protecting head/tail/count.
Allowing pool_submit to enqueue after shutdown.
```

---

### Exercise 4 Solution: Parallel Divide-and-Conquer with Reductions and Load Balancing

This problem tests:

```text
parallel algorithms
divide and conquer
thread pools
heap ownership
condition variables
task completion
reductions
load balancing
```

The major trap is this: a divide-and-conquer task cannot simply submit two children and immediately read their result. It must wait until both children finish.

One clean solution uses a small “future” object for each task.

```c
struct future_sum {
    pthread_mutex_t lock;
    pthread_cond_t cv;
    int done;
    long long value;
};
```

Future helpers:

```c
void future_init(struct future_sum *f) {
    pthread_mutex_init(&f->lock, NULL);
    pthread_cond_init(&f->cv, NULL);
    f->done = 0;
    f->value = 0;
}

void future_set(struct future_sum *f, long long value) {
    pthread_mutex_lock(&f->lock);

    f->value = value;
    f->done = 1;

    pthread_cond_broadcast(&f->cv);

    pthread_mutex_unlock(&f->lock);
}

long long future_get(struct future_sum *f) {
    pthread_mutex_lock(&f->lock);

    while (!f->done) {
        pthread_cond_wait(&f->cv, &f->lock);
    }

    long long value = f->value;

    pthread_mutex_unlock(&f->lock);

    return value;
}

void future_destroy(struct future_sum *f) {
    pthread_cond_destroy(&f->cv);
    pthread_mutex_destroy(&f->lock);
}
```

Task argument:

```c
#define THRESHOLD 4096

struct sum_task {
    struct pool *pool;
    int *data;
    int left;
    int right;
    struct future_sum *out;
};
```

Sequential helper:

```c
static long long seq_sum(int *data, int left, int right) {
    long long sum = 0;

    for (int i = left; i < right; i++) {
        sum += data[i];
    }

    return sum;
}
```

Task function:

```c
void sum_task_main(void *arg) {
    struct sum_task *t = arg;

    int len = t->right - t->left;

    if (len <= THRESHOLD) {
        long long sum = seq_sum(t->data, t->left, t->right);
        future_set(t->out, sum);
        free(t);
        return;
    }

    int mid = t->left + len / 2;

    struct future_sum left_future;
    struct future_sum right_future;

    future_init(&left_future);
    future_init(&right_future);

    struct sum_task *left_task = malloc(sizeof *left_task);
    struct sum_task *right_task = malloc(sizeof *right_task);

    *left_task = (struct sum_task) {
        .pool = t->pool,
        .data = t->data,
        .left = t->left,
        .right = mid,
        .out = &left_future
    };

    *right_task = (struct sum_task) {
        .pool = t->pool,
        .data = t->data,
        .left = mid,
        .right = t->right,
        .out = &right_future
    };

    pool_submit(t->pool, sum_task_main, left_task);
    pool_submit(t->pool, sum_task_main, right_task);

    long long a = future_get(&left_future);
    long long b = future_get(&right_future);

    future_destroy(&left_future);
    future_destroy(&right_future);

    future_set(t->out, a + b);

    free(t);
}
```

Top-level call:

```c
long long parallel_sum(struct pool *p, int *data, int n) {
    struct future_sum result;
    future_init(&result);

    struct sum_task *root = malloc(sizeof *root);

    *root = (struct sum_task) {
        .pool = p,
        .data = data,
        .left = 0,
        .right = n,
        .out = &result
    };

    pool_submit(p, sum_task_main, root);

    long long ans = future_get(&result);

    future_destroy(&result);

    return ans;
}
```

But this design has an important issue: **thread-pool deadlock**.

Suppose all worker threads are running tasks that submit subtasks and then block waiting for those subtasks. If no worker is free to run the subtasks, the pool deadlocks.

A safer exam answer should mention one of these fixes:

```text
1. Use a work-stealing scheduler.
2. Let waiting workers execute other queued tasks while waiting.
3. Submit only one subtask and compute the other half directly.
4. Limit recursive task creation depth.
```

A simpler safe version submits one side and computes the other side in the current worker:

```c
void sum_task_main_safe(void *arg) {
    struct sum_task *t = arg;

    int len = t->right - t->left;

    if (len <= THRESHOLD) {
        long long sum = seq_sum(t->data, t->left, t->right);
        future_set(t->out, sum);
        free(t);
        return;
    }

    int mid = t->left + len / 2;

    struct future_sum right_future;
    future_init(&right_future);

    struct sum_task *right_task = malloc(sizeof *right_task);

    *right_task = (struct sum_task) {
        .pool = t->pool,
        .data = t->data,
        .left = mid,
        .right = t->right,
        .out = &right_future
    };

    pool_submit(t->pool, sum_task_main_safe, right_task);

    /*
     * Compute left half directly in the current worker.
     */
    long long left_sum = seq_sum(t->data, t->left, mid);
    long long right_sum = future_get(&right_future);

    future_destroy(&right_future);

    future_set(t->out, left_sum + right_sum);

    free(t);
}
```

This sacrifices some parallelism but avoids the worst case where every worker blocks waiting for tasks that cannot run.

Most common mistakes:

```text
Creating a new pthread for every recursive call.
Returning a pointer to a local stack variable.
Writing to a shared global sum without synchronization.
Using too small a threshold, causing task overhead to dominate.
Ignoring thread-pool deadlock from nested futures.
Assuming divide-and-conquer automatically gives good load balancing.
```

---

### Exercise 5 Solution: Signal-Controlled `exec()` Runner with Timeout

This exercise combines:

```text
fork()
exec()
signals
alarm()
waitpid()
process lifecycle
file descriptor inheritance
async-signal safety
```

Global state:

```c
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

static volatile sig_atomic_t timed_out = 0;

static void on_alarm(int sig) {
    (void)sig;
    timed_out = 1;
}
```

Runner:

```c
int run_with_timeout(char *const argv[], int seconds) {
    struct sigaction sa;

    sa.sa_handler = on_alarm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGALRM, &sa, NULL);

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        execvp(argv[0], argv);

        /*
         * Reached only if execvp() fails.
         * Use _exit() in the child.
         */
        perror("execvp");
        _exit(127);
    }

    alarm(seconds);

    int status;

    for (;;) {
        pid_t r = waitpid(pid, &status, 0);

        if (r == pid) {
            alarm(0);

            if (WIFEXITED(status)) {
                printf("child exited with %d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("child killed by signal %d\n", WTERMSIG(status));
            }

            return status;
        }

        if (r == -1 && errno == EINTR) {
            if (timed_out) {
                break;
            }

            continue;
        }

        if (r == -1) {
            perror("waitpid");
            return -1;
        }
    }

    /*
     * Timeout occurred.
     */
    kill(pid, SIGTERM);

    /*
     * Give child a short grace period.
     * In an exam answer, this can be described in pseudocode.
     */
    for (int i = 0; i < 10; i++) {
        pid_t r = waitpid(pid, &status, WNOHANG);

        if (r == pid) {
            printf("child terminated after SIGTERM\n");
            return status;
        }

        usleep(100000);
    }

    kill(pid, SIGKILL);
    waitpid(pid, &status, 0);

    printf("child killed after timeout\n");
    return status;
}
```

Why `execvp()` does not return on success:

```text
execvp() replaces the current process image with a new program.
The PID stays the same, but the old code, stack, heap, and globals are replaced.
Therefore, if execvp() succeeds, the next line in the old program is not executed.
```

Why `_exit()` in the child after failed `execvp()`:

```text
The child was created by fork(), so it may have inherited stdio buffers from the parent.
Calling exit() can flush those buffers again.
_exit() terminates the child immediately without running normal process cleanup handlers.
```

Why not `printf()` inside the signal handler:

```text
printf() is not async-signal-safe.
The handler should only set a volatile sig_atomic_t flag.
Normal code can check the flag and perform printing/cleanup.
```

File descriptors across `fork()` and `exec()`:

```text
After fork(), the child inherits copies of the parent's file descriptors.
After exec(), file descriptors usually remain open unless marked close-on-exec.
This is why shells can set up redirection with dup2() before exec().
```

Most common mistakes:

```text
Assuming exec creates a new process.
Forgetting to waitpid(), causing zombies.
Calling printf() or malloc() in a signal handler.
Not handling waitpid() returning -1 with errno == EINTR.
Using exit() instead of _exit() in the child after exec failure.
Forgetting that file descriptors survive exec unless closed or marked close-on-exec.
```

---

### Exercise 6 Solution: Shared Memory, `fork()`, Race Condition, and Virtual Memory Reasoning

Question 1:

```text
global_counter in the parent remains 0.
```

Reason:

```text
After fork(), each child has its own logical copy of global_counter.
Children increment their own copies.
The parent's global_counter is not modified.
```

Question 2:

There are 4 children, each incrementing `*shared` 100000 times.

Ideal value:

```text
4 * 100000 = 400000
```

Question 3:

`(*shared)++` is not atomic.

It is logically:

```text
load old value
add 1
store new value
```

Two processes can interleave:

```text
Process A reads 10
Process B reads 10
Process A writes 11
Process B writes 11
```

One increment is lost.

Question 4: fixed version.

```c
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

struct shared_state {
    sem_t lock;
    int counter;
};

int main(void) {
    struct shared_state *s = mmap(NULL, sizeof *s,
                                  PROT_READ | PROT_WRITE,
                                  MAP_SHARED | MAP_ANONYMOUS,
                                  -1, 0);

    if (s == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    s->counter = 0;

    sem_init(&s->lock, 1, 1);

    for (int i = 0; i < 4; i++) {
        if (fork() == 0) {
            for (int j = 0; j < 100000; j++) {
                sem_wait(&s->lock);
                s->counter++;
                sem_post(&s->lock);
            }

            _exit(0);
        }
    }

    while (wait(NULL) > 0) {
    }

    printf("shared = %d\n", s->counter);

    sem_destroy(&s->lock);
    munmap(s, sizeof *s);

    return 0;
}
```

This version is correct but slow because it locks once per increment. A better version reduces locally:

```c
if (fork() == 0) {
    int local = 0;

    for (int j = 0; j < 100000; j++) {
        local++;
    }

    sem_wait(&s->lock);
    s->counter += local;
    sem_post(&s->lock);

    _exit(0);
}
```

Question 5: copy-on-write.

```text
After fork(), parent and child initially share physical pages marked copy-on-write.
If neither writes, both can map the same physical memory.
When a child writes to global_counter, the OS gives that child a private copy of the page.
Therefore the parent's global_counter is unaffected.
```

The `mmap(... MAP_SHARED ...)` region is different:

```text
It is intentionally shared.
Writes by one process are visible to the others.
But visibility does not imply atomicity.
```

Most common mistakes:

```text
Saying global_counter becomes 400000 in the parent.
Thinking MAP_SHARED makes ++ atomic.
Using sem_init(&sem, 0, 1) instead of sem_init(&sem, 1, 1).
Forgetting that fork creates separate address spaces.
Locking too frequently and not discussing performance.
```

---

### Exercise 7 Solution: Cache, Branch Prediction, and Parallel Matrix Reduction

Part A:

C arrays are row-major. Consecutive elements in the same row are adjacent in memory.

This loop has good spatial locality:

```c
for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
        s += A[i * cols + j];
    }
}
```

The memory access order is:

```text
A[0][0], A[0][1], A[0][2], ...
```

This is contiguous.

The second loop accesses:

```text
A[0][0], A[1][0], A[2][0], ...
```

The stride is:

```text
cols * sizeof(int)
```

If `cols` is large, this has poor cache locality.

So `sum1` is usually better.

Part B: parallel row-block sum.

```c
#include <pthread.h>
#include <stdlib.h>

struct worker_arg {
    int *A;
    int rows;
    int cols;
    int start_row;
    int end_row;
    long long result;
};

void *sum_rows(void *arg) {
    struct worker_arg *a = arg;

    long long local = 0;

    for (int i = a->start_row; i < a->end_row; i++) {
        for (int j = 0; j < a->cols; j++) {
            local += a->A[i * a->cols + j];
        }
    }

    a->result = local;
    return NULL;
}
```

Top-level:

```c
long long parallel_matrix_sum(int *A, int rows, int cols, int T) {
    pthread_t *threads = malloc(T * sizeof *threads);
    struct worker_arg *args = malloc(T * sizeof *args);

    for (int t = 0; t < T; t++) {
        int start = t * rows / T;
        int end = (t + 1) * rows / T;

        args[t].A = A;
        args[t].rows = rows;
        args[t].cols = cols;
        args[t].start_row = start;
        args[t].end_row = end;
        args[t].result = 0;

        pthread_create(&threads[t], NULL, sum_rows, &args[t]);
    }

    long long total = 0;

    for (int t = 0; t < T; t++) {
        pthread_join(threads[t], NULL);
        total += args[t].result;
    }

    free(args);
    free(threads);

    return total;
}
```

Part C: false sharing.

The field:

```c
args[t].result
```

for adjacent `t` values may sit on the same cache line. Multiple threads write to different `result` fields, but those fields may share one cache line. This causes cache-coherence invalidation traffic.

One fix is padding:

```c
struct worker_arg {
    int *A;
    int rows;
    int cols;
    int start_row;
    int end_row;
    long long result;
    char padding[64];
};
```

A more precise design would separate frequently written fields into cache-line-aligned per-thread storage.

Branch prediction note:

The inner loops here have predictable loop branches. The dominant performance issue is usually memory access order, not branch misprediction.

Most common mistakes:

```text
Parallelizing by columns and destroying cache locality.
Using one global sum protected by a mutex inside the inner loop.
Forgetting pthread_join() before reading results.
Passing &t as the thread argument.
Ignoring false sharing in per-thread result slots.
```

Bad parallel reduction:

```c
pthread_mutex_lock(&lock);
global_sum += A[i * cols + j];
pthread_mutex_unlock(&lock);
```

This serializes the inner loop and is usually worse than sequential code.

Good pattern:

```text
Each thread computes a local sum.
Main thread combines local sums after join.
```

---

### Exercise 8 Solution: Security, System Calls, and File-Copy Robustness

This problem combines:

```text
system calls
files
security
C buffers
file descriptors
error handling
```

Implementation:

```c
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

static int write_all(int fd, const void *buf, size_t n) {
    const char *p = buf;

    while (n > 0) {
        ssize_t w = write(fd, p, n);

        if (w == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }

        if (w == 0) {
            return -1;
        }

        p += w;
        n -= (size_t)w;
    }

    return 0;
}
```

Copy function:

```c
int copy_file(const char *src, const char *dst) {
    int in = -1;
    int out = -1;
    char buf[4096];

    in = open(src, O_RDONLY);

    if (in == -1) {
        return -1;
    }

    /*
     * O_NOFOLLOW helps avoid writing through a symlink.
     * O_EXCL prevents overwriting an existing file.
     */
    out = open(dst,
               O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW,
               0600);

    if (out == -1) {
        close(in);
        return -1;
    }

    for (;;) {
        ssize_t r = read(in, buf, sizeof buf);

        if (r == -1) {
            if (errno == EINTR) {
                continue;
            }

            close(in);
            close(out);
            return -1;
        }

        if (r == 0) {
            break;
        }

        if (write_all(out, buf, (size_t)r) == -1) {
            close(in);
            close(out);
            return -1;
        }
    }

    int ok = 0;

    if (close(in) == -1) {
        ok = -1;
    }

    if (close(out) == -1) {
        ok = -1;
    }

    return ok;
}
```

Why `read/write` are different from `FILE *` functions:

```text
read() and write() operate on file descriptors and raw bytes.
fgets(), fprintf(), fscanf(), and fclose() operate on FILE * streams and include standard-library buffering.
```

Why partial writes matter:

```text
write(fd, buf, n) is allowed to write fewer than n bytes.
This is common for pipes, sockets, terminals, interrupted calls, and nonblocking descriptors.
A robust program loops until all bytes are written.
```

Why `O_NOFOLLOW` helps:

```text
If dst is a symlink, opening it may write to the symlink target.
An attacker could redirect output to a sensitive file.
O_NOFOLLOW makes open fail if the final path component is a symlink.
```

Buffer overflow issue:

Bad code:

```c
char buf[128];
strcpy(buf, user_path);
```

If `user_path` is longer than 127 characters plus `'\0'`, it overwrites adjacent stack memory. This can corrupt local variables, saved registers, return addresses, or control flow.

Most common mistakes:

```text
Assuming write() writes all bytes.
Using sprintf/strcpy on fixed-size buffers without bounds.
Forgetting to close one fd when the second open fails.
Creating destination with overly broad permissions.
Ignoring symlink attacks.
Mixing FILE * and file descriptors incorrectly.
```

---

### Exercise 9 Solution: Compiler/Linker, Static Symbols, Shared Libraries, and Process Memory

Question 1:

```text
static at file scope gives internal linkage.
The symbol counter is visible only inside counter.c.
Other translation units cannot directly refer to counter by name.
```

The functions:

```c
void inc(void);
int get(void);
```

have external linkage and can be called from `main.c`.

Question 2:

Before `fork()`:

```text
counter = 2
```

After `fork()`:

```text
child has its own logical copy of counter = 2
parent has its own logical copy of counter = 2
```

Child calls:

```c
inc();
```

so child prints:

```text
child: 3
```

Parent waits, but its own `counter` remains 2, so parent prints:

```text
parent: 2
```

The likely output is:

```text
child: 3
parent: 2
```

The parent waits, so child output comes first.

Question 3:

```text
No.
The child increment does not affect the parent because fork() creates a separate virtual address space.
```

Question 4: static library build.

```bash
gcc -c counter.c -o counter.o
ar rcs libcounter.a counter.o
gcc -c main.c -o main.o
gcc main.o -L. -lcounter -o prog
```

A static library is copied into the executable at link time.

Question 5: shared library build.

```bash
gcc -fPIC -c counter.c -o counter.o
gcc -shared -o libcounter.so counter.o
gcc main.c -L. -lcounter -Wl,-rpath=. -o prog
```

A shared object is loaded dynamically at run time.

Question 6: program regions.

```text
Program code/text:
    compiled machine code for main(), inc(), get()

Static/global region:
    static int counter

Stack:
    main() local variables, such as pid

Heap:
    none used in this program unless library/runtime uses heap internally
```

Copy-on-write:

```text
After fork(), parent and child may initially map the same physical page containing counter.
When the child writes counter, the OS gives the child a private copy of that page.
The parent still sees counter = 2.
```

Most common mistakes:

```text
Thinking static means the variable is stored on the stack.
Thinking the child modifies the parent's static variable.
Confusing preprocessing, compiling, assembling, and linking.
Thinking a shared library means all processes share writable global variables by default.
Forgetting that internal linkage affects symbol visibility, not lifetime.
```

---

### Exercise 10 Solution: Cross-Topic Debugging: Race, Deadlock, and Memory Lifetime

Part A: deadlock.

Suppose two threads run:

```text
Thread 1: transfer(A, B)
Thread 2: transfer(B, A)
```

Interleaving:

```text
Thread 1 locks A.
Thread 2 locks B.
Thread 1 waits for B.
Thread 2 waits for A.
```

Neither can continue.

This is circular wait.

Part B: fix with global lock ordering.

```c
void transfer(struct account *from, struct account *to, int amount) {
    struct account *first = from;
    struct account *second = to;

    if (first->id > second->id) {
        first = to;
        second = from;
    }

    pthread_mutex_lock(&first->lock);

    if (second != first) {
        pthread_mutex_lock(&second->lock);
    }

    if (from->balance >= amount) {
        from->balance -= amount;
        to->balance += amount;
    }

    if (second != first) {
        pthread_mutex_unlock(&second->lock);
    }

    pthread_mutex_unlock(&first->lock);
}
```

Why this works:

```text
Every thread acquires account locks in increasing account-id order.
Therefore circular wait cannot occur.
```

Part C: thread argument lifetime bug.

Buggy code:

```c
for (int i = 0; i < nthreads; i++) {
    struct job job;
    ...
    pthread_create(&tids[i], NULL, worker, &job);
}
```

`job` is a stack variable inside the loop body. Each iteration creates a variable whose lifetime ends at the end of the iteration. The thread receives a pointer to an object that may no longer exist.

Also, even if the compiler reuses the same stack address, all threads may observe overwritten values.

Correct version using an array:

```c
struct job *jobs = malloc(nthreads * sizeof *jobs);

for (int i = 0; i < nthreads; i++) {
    jobs[i].from = &accounts[i % naccounts];
    jobs[i].to = &accounts[(i + 1) % naccounts];
    jobs[i].amount = 10;

    pthread_create(&tids[i], NULL, worker, &jobs[i]);
}

for (int i = 0; i < nthreads; i++) {
    pthread_join(tids[i], NULL);
}

free(jobs);
```

Most common mistakes:

```text
Fixing the race but introducing deadlock.
Using trylock without a clear retry policy.
Passing a pointer to a loop-local stack variable.
Destroying account mutexes before all threads join.
Failing to handle transfer(A, A).
```
