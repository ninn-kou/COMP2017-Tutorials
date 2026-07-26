## COMP2017 26S1 Passway to Pass Workshop Part 2 Solutions

> [!IMPORTANT]
> Read the complete [question paper](./P2P_Workshop_Part_2_Exercises_Q) before using these solutions. They are only provided to you as the template for revision. The real exam could cover more contents/topics, and marking rubric is different.

### Contents

- [Solution 1 Deadlock (20 marks)](#solution-1-deadlock-20-marks)
  - [Q1.1 Deadlock Safety, Consequences, and Avoidance (8 marks)](#q11-deadlock-safety-consequences-and-avoidance-8-marks)
  - [Q1.2 Diagnose and Repair a Deadlock (12 marks)](#q12-diagnose-and-repair-a-deadlock-12-marks)
- [Solution 2 Programming: Parallel Image Statistics (30 marks)](#solution-2-programming-parallel-image-statistics-30-marks)
  - [Q2.1 Pthread Workers (14 marks)](#q21-pthread-workers-14-marks)
  - [Q2.2 Process Workers (16 marks)](#q22-process-workers-16-marks)

---

### Solution 1 Deadlock (20 marks)

#### Q1.1 Deadlock Safety, Consequences, and Avoidance (8 marks)

##### Step 1: Separate the three state descriptions

- A **safe state** has at least one order in which every task can receive its remaining maximum need, finish, and release its resources.
- An **unsafe state** has no such guaranteed completion order. It has the potential to reach deadlock, but tasks need not be blocked yet.
- A **deadlocked state** has a set of tasks that are already waiting indefinitely for events that only tasks in that set can cause.

The key distinction is this:

```text
safe       -> some guaranteed completion sequence exists
unsafe     -> no guaranteed completion sequence exists
deadlocked -> circularly dependent tasks are already unable to progress
```

##### Step 2: Run the safety check on the original state

Start with `work = available = (1, 0)`. A task can be selected when its maximum remaining need is no greater than `work` in every component.

| Step | Task selected | Need that can be met | `work` after its current allocation is released |
| ---: | --- | --- | --- |
| 0 | - | - | `(1, 0)` |
| 1 | `T1` | `(1, 0)` | `(1, 0) + (1, 1) = (2, 1)` |
| 2 | `T0` | `(1, 1)` | `(2, 1) + (1, 0) = (3, 1)` |
| 3 | `T2` | `(1, 0)` | `(3, 1) + (0, 1) = (3, 2)` |

Therefore, one safe sequence is:

```text
T1 -> T0 -> T2
```

The safety algorithm adds a finishing task's **current allocation** to `work`. Its remaining need can be borrowed from `work` and returned when it finishes, so that part cancels out.

##### Step 3: Check the proposed grant to `T0`

After the grant:

| Task | Current allocation | Maximum remaining need |
| --- | --- | --- |
| `T0` | `(2, 0)` | `(0, 1)` |
| `T1` | `(1, 1)` | `(1, 0)` |
| `T2` | `(0, 1)` | `(1, 0)` |

The available vector is `(0, 0)`. No task has a remaining need that can be met, so the safety check cannot choose even its first task. The new state is **unsafe**.

It is not necessarily already deadlocked. For example, a task may still be computing and may not yet have requested its declared maximum. If `T0` actually requests `B` while `T1` and `T2` actually request `A`, then none can proceed and the state becomes deadlocked.

> What does the safety algorithm promise? It promises a completion route even if every task later asks for its declared maximum; it does not claim that every unsafe snapshot already contains blocked tasks.

##### Step 4: Explain the consequence

Deadlock is a **liveness** failure: the program may preserve its data invariants, yet the affected work never makes progress.

Practical consequences include:

- requests hang and deadlines or timeouts are exceeded;
- locked resources remain unavailable, so unrelated work can queue behind the deadlocked tasks;
- throughput for the affected operation can fall to zero;
- the service may require operator intervention or a restart.

Any two well-explained consequences earn the consequence marks.

##### Step 5: Give one prevention method and one avoidance method

**Prevention:** assign every lock a rank and require all code to acquire locks in increasing rank order. This makes the **circular-wait** Coffman condition impossible.

**Avoidance:** before granting a request, tentatively allocate the resources and run a safety check. Grant the request only if the resulting state still has a safe completion sequence. Banker's algorithm is the standard example.

The decision rule is:

```text
prevention -> make a necessary deadlock condition structurally impossible
avoidance  -> inspect each proposed grant and remain in safe states
```

##### Marking guidance for Q1.1

| Evidence | Marks |
| --- | ---: |
| Correct safe, unsafe, and deadlocked definitions | 1.5 |
| A valid safety trace and safe sequence | 1.5 |
| Correct unsafe classification and distinction from an existing deadlock | 1 |
| Liveness explanation and two practical consequences | 2 |
| Prevention method, broken Coffman condition, avoidance method, and safety check | 2 |
| **Total** | **8** |

Common losses:

- saying "unsafe means deadlocked";
- adding a task's maximum claim, rather than its current allocation, to `work` after the task finishes;
- naming a mutex as a prevention method without explaining how all code uses it;
- calling detection-and-recovery an avoidance method.

---

#### Q1.2 Diagnose and Repair a Deadlock (12 marks)

For a compact trace, write:

```text
S = store_page thread     F = flush_page thread
C = cache_lock            J = journal_lock
```

##### Step 1: Construct a feasible deadlocking schedule

| Event | Action | `S` after event | `F` after event |
| ---: | --- | --- | --- |
| 1 | `S` completes line 7 | owns `C`; waits for nothing | owns nothing; waits for nothing |
| 2 | `F` completes line 17 | owns `C`; waits for nothing | owns `J`; waits for nothing |
| 3 | `S` calls line 9 and blocks | owns `C`; waits for `J` | owns `J`; waits for nothing |
| 4 | `F` calls line 19 and blocks | owns `C`; waits for `J` | owns `J`; waits for `C` |

Between these lock-state events, `update_cache()` at line 8 and `prepare_flush()` at line 18 complete. They do not change mutex ownership, so they are omitted from the four-event synchronization trace. The important detail is that the lock calls in events 3 and 4 do not return. Each blocked thread keeps the mutex it acquired earlier.

##### Step 2: Diagnose the final state

The ownership-and-waiting table is:

| Thread | Owns | Waits for | Owner of requested mutex |
| --- | --- | --- | --- |
| `S` | `C` | `J` | `F` |
| `F` | `J` | `C` | `S` |

A resource-allocation graph contains this cycle:

```text
C --owned by--> S --requests--> J --owned by--> F --requests--> C
```

Equivalently, the directed cycle is `C -> S -> J -> F -> C`.

All four Coffman conditions hold:

| Condition | Evidence in this execution |
| --- | --- |
| Mutual exclusion | Each ordinary mutex can be owned by only one thread. |
| Hold and wait | `S` holds `C` while requesting `J`; `F` holds `J` while requesting `C`. |
| No preemption | A mutex is released by its owner; the other thread cannot forcibly take it. |
| Circular wait | `S` waits for `F` through `J`, while `F` waits for `S` through `C`. |

##### Step 3: Confirm the diagnosis in a repeatable way

A general debugging procedure is:

1. Observe the program over time and confirm that the same threads remain blocked rather than merely running slowly.
2. Inspect the state of each blocked thread using a debugger, logging, or lock instrumentation.
3. Record which mutex each thread currently owns and which mutex it is waiting to acquire.
4. Construct the waiting relationship and check for a cycle.

For this program, the evidence would be:

```text
store_page: owns cache_lock, waits for journal_lock
flush_page: owns journal_lock, waits for cache_lock
```

This produces the cycle:

```text
store_page -> journal_lock -> flush_page
     ^                            |
     |                            v
cache_lock <-----------------------
```

Because neither thread can continue until the other releases its mutex—and neither can reach its unlock operations—the cycle confirms a deadlock.

**For GDB:**

Run the workload until the hang is observed and record the process ID. Attach GDB and inspect every thread rather than guessing from one stack:

```text
$ gdb -p <pid>
(gdb) info threads
(gdb) thread apply all bt
```

The backtraces should show both worker threads stopped in or below `pthread_mutex_lock()`. The first relevant caller frame should put:

- the `store_page` thread at line 9, trying to acquire `journal_lock`;
- the `flush_page` thread at line 19, trying to acquire `cache_lock`.

Then compare the requested mutexes with the locks already held along each path. Whenever this particular hang is observed, the stacks should reconstruct the same ownership cycle, even if thread IDs or low-level futex frames differ. A test-only barrier or debugger breakpoints can force the two first-lock acquisitions when a deterministic reproduction is needed.

> One blocked stack proves contention. Two stacks plus the ownership cycle prove this deadlock.

##### Step 4: Repair the lock order

Choose one global order:

```text
cache_lock before journal_lock
```

`store_page` already follows that order. Replace lines 17-22 in `flush_page` so that it follows the same order:

```c
pthread_mutex_lock(&cache_lock);
pthread_mutex_lock(&journal_lock);

prepare_flush(page);
evict_page(page);
```

Release the locks after the protected work. Reversing the acquisition order when unlocking is conventional and easy to audit:

```c
pthread_mutex_unlock(&journal_lock);
pthread_mutex_unlock(&cache_lock);
```

The repair preserves at least the original protection: `prepare_flush()` still runs while `journal_lock` is held, and `evict_page()` still runs while both locks are held. It may hold `cache_lock` for slightly longer, which is a performance trade-off rather than a correctness error.

See the [complete repaired program](Codes/deadlock_lock_order_fixed.c).

##### Step 5: Prove that circular wait is impossible

Assign ranks:

```text
rank(cache_lock) = 1
rank(journal_lock) = 2
```

Every thread can wait only for a mutex with a greater rank than every mutex it already holds. Following wait edges would therefore produce a strictly increasing sequence of ranks. A directed cycle would have to return to its starting rank, which is impossible. Circular wait cannot occur, so this two-lock deadlock is prevented.

##### Marking guidance for Q1.2

| Evidence | Marks |
| --- | ---: |
| Feasible four-event schedule with correct ownership and waits | 3 |
| Table, directed cycle, and evidence for all four Coffman conditions | 3 |
| Sensible investigation method, such as inspecting blocked threads and mutex states | 1 |
| Correct ownership/waiting cycle and explanation that neither thread can progress | 1 |
| Repeatable GDB procedure and expected backtrace evidence | Optional |
| Correct common acquisition order with protection preserved | 3 |
| Rank-based proof that circular wait is impossible | 1 |
| **Total** | **12** |

Common losses:

- claiming that changing only the unlock order fixes the deadlock;
- inserting `sleep()` and treating a less likely schedule as a proof of correctness;
- using `trylock()` without a complete release-and-retry protocol;
- fixing only one call site while another path still acquires the locks in the opposite order;
- asserting that the unlocks "will eventually happen" even though the threads cannot reach them.

---

### Solution 2 Programming: Parallel Image Statistics (30 marks)

Both implementations use the same worker calculation and the same combiner. Only the execution and result-transfer mechanisms change.

#### Shared Step 1: Compute one half-open range

An empty accumulator is the identity for this reduction: zero for both additions and zero for `max`, because every pixel is non-negative.

```c
static struct image_stats scan_range(const uint8_t *pixels,
        size_t begin, size_t end, uint8_t threshold) {
    struct image_stats stats = {0, 0, 0};
    for (size_t i = begin; i < end; i++) {
        const uint8_t value = pixels[i];
        stats.sum += value;
        stats.bright_count += value >= threshold;
        if (value > stats.maximum) stats.maximum = value;
    }
    return stats;
}
```

The combiner applies exactly the same reduction operations to partial records:

```c
static void combine(struct image_stats *total,
                    const struct image_stats *partial) {
    total->sum += partial->sum;
    total->bright_count += partial->bright_count;
    if (partial->maximum > total->maximum)
        total->maximum = partial->maximum;
}
```

#### Shared Step 2: Trace the supplied input

There are 17 pixels. Integer division produces complete, non-overlapping half-open ranges:

| Worker | Range | Values | Partial sum | Bright count | Partial maximum |
| ---: | --- | --- | ---: | ---: | ---: |
| 0 | `[0, 4)` | `12, 240, 88, 201` | 541 | 2 | 240 |
| 1 | `[4, 8)` | `199, 255, 0, 32` | 486 | 1 | 255 |
| 2 | `[8, 12)` | `220, 17, 198, 202` | 637 | 2 | 220 |
| 3 | `[12, 17)` | `64, 180, 230, 3, 99` | 576 | 1 | 230 |
| **Combined** | `[0, 17)` | all pixels | **2240** | **6** | **255** |

Expected output from either complete program:

```text
sum=2240 bright=6 max=255
```

> Why does worker 3 receive five pixels? The range formula distributes the remainder while still covering every index exactly once.

---

#### Q2.1 Pthread Workers (14 marks)

##### Step 1: Give each thread a stable task record

The task record lives in `main()` until after every join, so a worker never receives a pointer to an expired loop-local object.

```c
struct thread_task {
    const uint8_t *pixels;
    size_t begin;
    size_t end;
    uint8_t threshold;
    struct image_stats result;
};
```

The worker has the POSIX-required signature. It writes only its own result field.

```c
static void *scan_worker(void *argument) {
    struct thread_task *task = argument;
    task->result = scan_range(task->pixels, task->begin,
                              task->end, task->threshold);
    return NULL;
}
```

##### Step 2: Create the complete worker set

Calculate ranges from the worker index. Do not pass `&i` to every worker and do not hard-code the four partitions.

```c
tasks[i] = (struct thread_task) {
    .pixels = pixels,
    .begin = pixel_count * i / WORKERS,
    .end = pixel_count * (i + 1) / WORKERS,
    .threshold = BRIGHT_THRESHOLD,
    .result = {0, 0, 0}
};
```

Create all possible workers before the join phase. `created` is both the cleanup boundary and the completeness check.

```c
int error = pthread_create(&threads[i], NULL,
                           scan_worker, &tasks[i]);
if (error != 0) {
    fprintf(stderr, "pthread_create: %s\n", strerror(error));
    failed = 1;
    break;
}
created++;
```

Putting `pthread_join()` here would wait for each worker immediately and serialize the program: create worker 0, wait for worker 0, then create worker 1, and so on.

##### Step 3: Join every worker that was actually created

Even after a later creation failure, earlier threads are still running resources owned by this process. Join every one of them.

```c
for (size_t i = 0; i < created; i++) {
    int error = pthread_join(threads[i], NULL);
    if (error != 0) {
        fprintf(stderr, "pthread_join: %s\n", strerror(error));
        failed = 1;
    }
}
```

Do not publish a plausible-looking answer assembled from only the workers that happened to start:

```c
if (failed || created != WORKERS) {
    fprintf(stderr, "incomplete worker set; result rejected\n");
    return EXIT_FAILURE;
}
```

##### Step 4: Combine only after successful joins

`pthread_join()` is the hand-off point: after it succeeds, the parent thread can read that worker's completed private record.

```c
struct image_stats total = {0, 0, 0};
for (size_t i = 0; i < WORKERS; i++) {
    combine(&total, &tasks[i].result);
}
```

No mutex is needed for `tasks[i].result` because:

1. exactly one worker writes each record;
2. different workers write different records;
3. the main thread reads a record only after joining its writer;
4. the input pixels are read-only.

This is ownership-based synchronization: private writes followed by a join, rather than shared updates protected by a lock.

See the [complete pthread solution](./Codes/image_stats_pthreads.c).

##### Marking guidance for Q2.1

| Evidence | Marks |
| --- | ---: |
| Stable task-record fields, lifetime, and private-result ownership | 2 |
| Correct worker signature, formula-derived range, and local scan | 2 |
| Complete create phase with stable arguments and creation-failure tracking | 3 |
| Join every created thread and reject an incomplete set | 2 |
| Correct addition/max combination and output | 2 |
| Correct no-mutex ownership/join explanation | 1 |
| Correct four-range trace and final values | 2 |
| **Total** | **14** |

Common losses:

- passing the address of one changing loop variable to all threads;
- joining inside the creation loop and losing useful parallelism;
- letting every worker update one shared total without synchronization;
- forgetting to join already-created threads after a later `pthread_create()` failure;
- adding partial maxima instead of taking their maximum;
- using inclusive endpoints and counting a boundary pixel twice.

---

#### Q2.2 Process Workers (16 marks)

##### Step 1: Choose an explicit result path

After `fork()`, parent and child have separate virtual address spaces. Copy-on-write may initially share physical pages, but a child's assignment changes its own logical copy. It does not update an ordinary variable in the parent.

Therefore, each child sends one `struct image_stats` through a pipe:

```text
pixels (inherited read-only) -> child scans range
child's private record       -> pipe bytes -> parent's partial[w]
parent partial records       -> combiner   -> final record
```

##### Step 2: Create every result pipe before forking

One pipe per intended worker makes ownership and result indexing explicit.

```c
int result_pipe[WORKERS][2];
for (size_t i = 0; i < WORKERS; i++) {
    if (pipe(result_pipe[i]) < 0) {
        perror("pipe");
        /* close every descriptor opened so far */
        return EXIT_FAILURE;
    }
}
```

The complete program closes all earlier pipe descriptors on this error path. Creating all pipes first also means every child inherits all descriptors, so descriptor cleanup in the child is essential.

##### Step 3: Fork workers and record their PIDs

The parent stores only successfully created child PIDs. A child immediately enters the child-only path and never returns to the parent's fork loop.

```c
pid_t pid = fork();
if (pid < 0) {
    perror("fork");
    spawn_ok = false;
    break;
}
if (pid == 0)
    run_child(i, result_pipe, pixels, pixel_count);
children[i] = pid;
created++;
```

On partial failure, `created` tells the parent which pipes to drain and which PIDs to reap. The final success gate rejects the incomplete result set.

##### Step 4: Keep only the child's own write end

Child `w` closes every read end and every other child's write end:

```c
for (size_t i = 0; i < WORKERS; i++) {
    close(result_pipe[i][0]);
    if (i != worker_index)
        close(result_pipe[i][1]);
}
```

It calculates its formula-derived range and writes one complete record:

```c
size_t begin = pixel_count * worker_index / WORKERS;
size_t end = pixel_count * (worker_index + 1) / WORKERS;
struct image_stats part =
    scan_range(pixels, begin, end, BRIGHT_THRESHOLD);
int ok = write_full(result_pipe[worker_index][1],
                    &part, sizeof part);
```

Finally, the child closes its only remaining descriptor and uses `_exit()` so it does not run parent cleanup or flush inherited stdio buffers:

```c
close(result_pipe[worker_index][1]);
_exit(ok == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
```

`write_full()` is required because a robust program must not assume one `write()` transfers every requested byte. It retries `EINTR` and advances through any partial transfer.

##### Step 5: Make the parent a reader only

> After `fork()`, which descriptors should the parent keep, and which single descriptor should child `w` keep?

After the fork loop, the parent closes all write ends. It also closes read ends for workers that were never created.

```c
for (size_t i = 0; i < WORKERS; i++) {
    close(result_pipe[i][1]);
    if (i >= created)
        close(result_pipe[i][0]);
}
```

Now drain every created child's record **before** entering the wait loop:

```c
for (size_t i = 0; i < created; i++) {
    if (read_full(result_pipe[i][0], &partial[i],
                  sizeof partial[i]) < 0)
        data_ok = false;
    close(result_pipe[i][0]);
}
```

Closing unused write ends matters: otherwise an incomplete writer can disappear while another process still holds a write descriptor, and the reader may wait forever instead of observing EOF.

##### Step 6: Reap and validate every created child

Use a small wrapper that retries `waitpid()` after `EINTR`:

```c
static pid_t waitpid_nointr(pid_t pid, int *status) {
    pid_t result;
    do {
        result = waitpid(pid, status, 0);
    } while (result < 0 && errno == EINTR);
    return result;
}
```

Then validate normal, successful termination for every recorded PID:

```c
for (size_t i = 0; i < created; i++) {
    int status;
    pid_t r = waitpid_nointr(children[i], &status);
    if (r != children[i] || !WIFEXITED(status)
        || WEXITSTATUS(status) != EXIT_SUCCESS)
        children_ok = false;
}
```

`waitpid()` transfers lifecycle information: which child changed state and how it terminated. The exit status is a small success/failure code. Neither operation copies the `struct image_stats`; the pipe does that.

##### Step 7: Publish only a complete result

All three dimensions of success must hold: the workers existed, their data arrived, and they terminated successfully.

```c
if (!spawn_ok || created != WORKERS
    || !data_ok || !children_ok) {
    fprintf(stderr, "incomplete worker set; result rejected\n");
    return EXIT_FAILURE;
}
```

Only then combine the four records with the same `combine()` function used conceptually in Q2.1.

##### Step 8: Explain why reads precede waits

A pipe has finite capacity. In a more general version, a child may need to write a result larger than the remaining pipe space:

```text
parent waits for child exit
       ^                 |
       |                 v
child waits for parent to drain a full pipe
```

That creates a progress cycle. Draining result pipes before waiting lets producers finish writing and then exit. The fixed-size record here is small, but the order is a robust design rule and is required by the question.

See the [complete process solution](./Codes/image_stats_processes.c).

##### Marking guidance for Q2.2

| Evidence | Marks |
| --- | ---: |
| Correct process-memory explanation and explicit IPC choice | 1 |
| Four pipes created before forking, with setup-failure cleanup | 2 |
| Fork loop, PID recording, and partial-creation tracking | 2 |
| Child descriptor discipline, formula range, full write, close, and `_exit()` | 3 |
| Parent descriptor discipline and complete reads before waits | 3 |
| `EINTR`-safe `waitpid()`, status validation, and reaping of all created children | 2 |
| Complete-result gate and correct combination | 1 |
| Correct explanations of result transport and pipe progress risk | 2 |
| **Total** | **16** |

Common losses:

- expecting a child's assignment to change a normal parent variable;
- creating one shared pipe but failing to frame or associate results with workers;
- leaving unrelated pipe write ends open;
- assuming one `read()` or `write()` transfers a complete record;
- calling `wait()` or `waitpid()` before draining data that a child may still be writing;
- treating an exit status as a general-purpose result channel;
- failing to reap already-created children after a later `fork()` failure;
- combining partial data even though a child, transfer, or status check failed.

---

### Final decision checklist

Before considering either program complete, ask:

1. Does every input index belong to exactly one half-open range?
2. Does every worker write only private result state?
3. Is there an explicit hand-off from worker to combiner: `pthread_join()` for threads, pipe transfer plus `waitpid()` validation for processes?
4. Are all successfully created execution units joined or reaped on failure paths?
5. Is an incomplete result set rejected rather than printed?
6. Are additive fields added and maximum fields compared?

If all six answers are yes, the worker-combiner structure is usually sound.
