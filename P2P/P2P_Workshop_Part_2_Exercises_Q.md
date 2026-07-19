## COMP2017 26S1 Passway to Pass Workshop Part 2 Exercises

> [!IMPORTANT]
> These are original topic-based practice exercises. They are **not** official or predicted exam questions. You must solve them by yourself, as knowledges will not transmitting to you mind just because you read the solution. **Otherwise you will fail again.**

### Contents

- [COMP2017 26S1 Passway to Pass Workshop Part 2 Exercises](#comp2017-26s1-passway-to-pass-workshop-part-2-exercises)
  - [Contents](#contents)
  - [Question 1 Deadlock (20 marks)](#question-1-deadlock-20-marks)
    - [Q1.1 Deadlock Safety, Consequences, and Avoidance (8 marks)](#q11-deadlock-safety-consequences-and-avoidance-8-marks)
    - [Q1.2 Diagnose and Repair a Deadlock (12 marks)](#q12-diagnose-and-repair-a-deadlock-12-marks)
  - [Question 2 Programming: Parallel Image Statistics (30 marks)](#question-2-programming-parallel-image-statistics-30-marks)
    - [Q2.1 Pthread Workers (14 marks)](#q21-pthread-workers-14-marks)
    - [Q2.2 Process Workers (16 marks)](#q22-process-workers-16-marks)

---

### Question 1 Deadlock (20 marks)

#### Q1.1 Deadlock Safety, Consequences, and Avoidance (8 marks)

A system has three reusable instances of resource `A` and two reusable instances of resource `B`. The resources are exclusive: one instance can be allocated to only one task at a time. An allocated resource cannot be forcibly taken away.

The current state is:

| Task | Allocated `A` | Allocated `B` | Maximum Remaining Need `A` | Maximum Remaining Need `B` |
| --- | ---: | ---: | ---: | ---: |
| `T0` | 1 | 0 | 1 | 1 |
| `T1` | 1 | 1 | 1 | 0 |
| `T2` | 0 | 1 | 1 | 0 |

The currently available vector is:

```text
available = (A = 1, B = 0)
```

Answer all parts.

1. Define a **safe state**, an **unsafe state**, and a **deadlocked state**. Find one safe completion sequence for the state above. **(3 marks)**
2. Suppose the final available instance of `A` is granted to `T0`. The new available vector is `(0, 0)`, and `T0` then has a maximum remaining need of `(0, 1)`. Classify the new state. Explain why an unsafe state is not necessarily already deadlocked. **(1 mark)**
3. Explain why deadlock is a liveness failure and give two practical consequences for a running program or service. **(2 marks)**
4. Describe one deadlock-prevention technique and one deadlock-avoidance technique. For prevention, state which Coffman condition is made impossible. For avoidance, state what must be checked before a resource request is granted. **(2 marks)**

#### Q1.2 Diagnose and Repair a Deadlock (12 marks)

A storage service protects its memory cache and write-ahead journal with two ordinary, non-recursive POSIX mutexes. One thread stores a page while another thread flushes a page.

The line numbers are provided for reference.

```c
01  static pthread_mutex_t cache_lock =
02      PTHREAD_MUTEX_INITIALIZER;
03  static pthread_mutex_t journal_lock =
04      PTHREAD_MUTEX_INITIALIZER;
05
06  static void *store_page(void *page) {
07      pthread_mutex_lock(&cache_lock);
08      update_cache(page);
09      pthread_mutex_lock(&journal_lock);
10      append_journal(page);
11      pthread_mutex_unlock(&journal_lock);
12      pthread_mutex_unlock(&cache_lock);
13      return NULL;
14  }
15
16  static void *flush_page(void *page) {
17      pthread_mutex_lock(&journal_lock);
18      prepare_flush(page);
19      pthread_mutex_lock(&cache_lock);
20      evict_page(page);
21      pthread_mutex_unlock(&cache_lock);
22      pthread_mutex_unlock(&journal_lock);
23      return NULL;
24  }
```

Assume that:

- both mutexes are initially unlocked;
- the helper functions terminate and do not acquire either mutex;
- each helper accesses only state protected by the mutexes held at that point;
- these are the only two routines that acquire both mutexes.

Answer all parts.

1. Give a feasible four-event execution schedule that leaves both threads permanently blocked. Use line numbers. After each event, state which mutex each thread owns and which mutex it is waiting for. **(3 marks)**
2. Diagnose the final state by giving:
   - an ownership-and-waiting table;
   - a wait-for or resource-allocation graph containing a directed cycle;
   - the four Coffman conditions and the evidence for each condition in this execution. **(3 marks)**
3. Describe, in general terms, how you would investigate a program that appears to have stopped making progress and determine whether deadlock is the cause. State what evidence about the threads, mutex ownership, and waiting relationships would confirm this deadlock. **(2 marks)** Describe a repeatable debugging procedure by using GDB for confirming this deadlock in a hung process. State what `thread apply all bt` in GDB should reveal. **(Optional)**
4. Repair the program while keeping both mutexes and preserving at least the original protection of every helper. Identify the affected lines and show the corrected lock and unlock operations. Do not use sleeps, cancellation, timeouts, or recursive mutexes. **(3 marks)**
5. Give a short proof that the repair prevents circular wait. Merely stating that the unlock calls will eventually run is not sufficient. **(1 mark)**

---

### Question 2 Programming: Parallel Image Statistics (30 marks)

A grayscale image is stored as a one-dimensional array. Each element is one pixel intensity in the range 0 to 255.

```c
#include <stddef.h>
#include <stdint.h>

enum {
    WORKERS = 4,
    BRIGHT_THRESHOLD = 200
};

struct image_stats {
    uint64_t sum;
    uint64_t bright_count;
    uint8_t maximum;
};

static const uint8_t pixels[] = {
    12, 240, 88, 201, 199, 255, 0, 32, 220,
    17, 198, 202, 64, 180, 230, 3, 99
};
```

For the complete image, calculate:

1. the sum of all pixel intensities;
2. the number of bright pixels, where a pixel is bright when its value is at least `BRIGHT_THRESHOLD`;
3. the maximum pixel intensity.

Worker `w` must process the half-open range:

```text
begin = pixel_count * w       / WORKERS
end   = pixel_count * (w + 1) / WORKERS
```

The worker processes indices `begin` through `end - 1`. Do not hard-code the ranges or partial answers.

For both parts, assume that:

- `pixel_count >= WORKERS`;
- the input array is read-only;
- all arithmetic fits in the supplied result types;
- a complete answer includes appropriate headers, declarations, cleanup, and error checks;
- if worker creation stops early, every successfully created worker must still be joined or reaped, and the incomplete result set must not be published.

You may define `read_full()` and `write_full()` helpers with this contract:

```text
return 0 only after exactly the requested number of bytes is transferred;
retry after EINTR;
return -1 on another error or on EOF before a complete record is read.
```

#### Q2.1 Pthread Workers (14 marks)

Write a complete C program that calculates the image statistics with four pthread workers.

Your answer must:

1. define a stable per-worker task record containing the input, half-open range, threshold, and private result;
2. implement a worker with the required `void *(*)(void *)` signature;
3. calculate every range using the supplied formula;
4. create the complete worker set before joining any worker;
5. join every successfully created worker;
6. reject an incomplete worker set;
7. combine the partial sums and counts by addition and the partial maxima with `max`;
8. explain why the private result fields require no mutex in this design;
9. give the four ranges, partial results, and final result for the supplied input.

Do not place `pthread_join()` inside the creation loop. **(14 marks)**

#### Q2.2 Process Workers (16 marks)

Write a second complete C program that calculates the same result with four child processes.

Your answer must:

1. explain why assigning a partial result to an ordinary child variable does not update the parent's variable;
2. create one anonymous result pipe per intended child before the calls to `fork()`;
3. record every successfully created child PID;
4. make child `w` retain only the write end of its own result pipe;
5. make the parent retain only the read end of every created child's result pipe;
6. make each child calculate one private `struct image_stats`, send one complete result record, close its descriptor, and terminate with `_exit()`;
7. make the parent read every created child's result record before entering its `waitpid()` loop;
8. retry interrupted transfers and waits;
9. reap and validate every created child with `waitpid()`;
10. publish the combined result only when all four children were created, all four records were complete, and all four children exited successfully;
11. explain why `waitpid()` and the child's exit status are not the result-transfer mechanism;
12. explain the progress risk in waiting for a producer before draining a potentially full pipe.

Use the same range formula and combination rule as Q2.1. **(16 marks)**
