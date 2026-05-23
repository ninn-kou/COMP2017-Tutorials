## COMP2017 2026 S1 Week 12 Tutorial B

<table><tbody>
  <tr><td><b>Tutor</b></td><td>Hao Ren</td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au">hao.ren@sydney.edu.au</a></td></tr>
</tbody></table>

- [COMP2017 2026 S1 Week 12 Tutorial B](#comp2017-2026-s1-week-12-tutorial-b)
  - [B.1 Recursion \& Recursion Overhead](#b1-recursion--recursion-overhead)
    - [B.1.1 Recursion Overhead](#b11-recursion-overhead)
    - [B.1.2 Stack Overflow](#b12-stack-overflow)
    - [B.1.3 Iterative Version](#b13-iterative-version)
    - [B.1.4 Important Distinction: Recursive Idea vs Recursive Implementation](#b14-important-distinction-recursive-idea-vs-recursive-implementation)
    - [B.1.5 Tail recursion](#b15-tail-recursion)
  - [B.2 Exercise: Fibonacci Numbers - I](#b2-exercise-fibonacci-numbers---i)
    - [B.2.1 Explanations to Solution](#b21-explanations-to-solution)
    - [B.2.2 Experiments: Measuring Time](#b22-experiments-measuring-time)
  - [B.3 Exercise: Fibonacci Numbers - II](#b3-exercise-fibonacci-numbers---ii)
    - [B.3.1 Step 1: Sequential Fibonacci Function](#b31-step-1-sequential-fibonacci-function)
    - [B.3.2 Step 2: Worker Thread for a Fibonacci Subproblem](#b32-step-2-worker-thread-for-a-fibonacci-subproblem)
    - [B.3.3 Step 3: T1 Combines the Results](#b33-step-3-t1-combines-the-results)
    - [B.3.4 Experiments: Measuring Optimization](#b34-experiments-measuring-optimization)
  - [B.4 Exercise: Fibonacci Numbers - III](#b4-exercise-fibonacci-numbers---iii)
    - [B.4.1 Step 1: Sequential Fibonacci Helper](#b41-step-1-sequential-fibonacci-helper)
    - [B.4.2 Step 2: Fully Threaded Version Using Semaphores](#b42-step-2-fully-threaded-version-using-semaphores)
    - [B.4.3 Maximum Number of Threads in the Naive Version](#b43-maximum-number-of-threads-in-the-naive-version)
    - [B.4.4 Optimised Version: Limit the Number of Worker Threads](#b44-optimised-version-limit-the-number-of-worker-threads)
    - [B.4.5 Is this faster? A Real-World Optimization Needed](#b45-is-this-faster-a-real-world-optimization-needed)

---

### B.1 Recursion & Recursion Overhead

Recursion is a way to solve a problem by reducing it into a smaller version of the same problem.

A recursive function usually has two parts:

```text
base case       stop condition
recursive step  call the same function on a smaller problem
```

The base case is important because recursion must eventually stop. Without a base case, or if the recursive step does not move toward the base case, the function keeps calling itself until the stack overflows.

When a function calls itself, it can move one step forward each time. In that sense, recursion is similar to mathematical induction, perhaps working from the opposite direction.

And for example, we could use the recursive method to calculate the sum of an array, where the `if` condition is the base case.

```c
int sum(int *arr, int length) {
    if (length == 0) {
        return 0;
    }

    return arr[0] + sum(arr + 1, length - 1);
}
```

```text
sum([3, 4, 5])
= 3 + sum([4, 5])
= 3 + 4 + sum([5])
= 3 + 4 + 5
= 12
```

Recursion is often natural when the problem itself has a recursive structure.

Examples include:

```text
walking a linked list
walking a tree
binary search
divide-and-conquer algorithms
backtracking
filesystem traversal
```

For example, a linked list is naturally recursive:

```text
a list is either empty
or a node followed by another list
```

A binary tree is also recursive:

```text
a tree is either empty
or a node with a left subtree and right subtree
```

Recursive code can be very clear for trees which naturally has recursive structure.

```c
void print_tree(struct node *root) {
    if (root == NULL) {
        return;
    }

    print_tree(root->left);
    printf("%d\n", root->value);
    print_tree(root->right);
}
```

The function handles one node, then recursively handles the smaller subtrees.

#### B.1.1 Recursion Overhead

Recursion is elegant, but it has a cost. Every function call creates a new stack frame. A stack frame stores information needed for that call, such as:

```text
function parameters
local variables
return address
saved registers
temporary state
```

So with recursion, every recursive call adds another frame to the stack. For example:

```c
int factorial(int n) {
    if (n <= 1) {
        return 1;
    }

    return n * factorial(n - 1);
}
```

Calling `factorial(4);` creates calls like:

```c
factorial(4)
factorial(3)
factorial(2)
factorial(1)
```

Before factorial(1) returns, the earlier calls are still waiting. Conceptually, the stack looks like:

```text
top
factorial(1)
factorial(2)
factorial(3)
factorial(4)
bottom
```

Then the calls return in reverse order:

```text
factorial(1) returns 1
factorial(2) returns 2 * 1
factorial(3) returns 3 * 2
factorial(4) returns 4 * 6
```

#### B.1.2 Stack Overflow

The program stack has limited space. If recursion goes too deep, too many stack frames are created. Eventually the stack can overflow, often causing a segmentation fault.

Example:

```c
void bad_recursion(void) {
    bad_recursion();
}
```

This function has no base case, so it keeps calling itself forever until the stack runs out.

**Even with a valid base case, recursion can still be too deep**.

```c
int sum(int *arr, int length) {
    if (length == 0) {
        return 0;
    }

    return arr[0] + sum(arr + 1, length - 1);
}
```

This works logically, but if `length` is extremely large, it may use too much stack.

> [!NOTE]
> For example, try running the command `./fib 1000000` in our exercise, [Fibonacci Number - I](#b2-exercise-fibonacci-numbers---i). You should see `Segmentation fault (core dumped)` almost immediately, before the program can perform any meaningful calculation.
>
> Then, try running `./fib 500`. This command may look like it is stuck, but it is actually still trying to compute the result.

> [!CAUTION]
> The number of function calls grows extremely quickly. For example, `fib(9)` calls `fib()` `109` times, while `fib(10)` calls it `177` times! We will introduce about these numbers with more details in exercise [Fibonacci Number - I](#b2-exercise-fibonacci-numbers---i).

#### B.1.3 Iterative Version

The same array sum can be written with a loop:

```c
int sum_iterative(int *arr, int length) {
    int total = 0;

    for (int i = 0; i < length; i++) {
        total += arr[i];
    }

    return total;
}
```

This version uses **one** stack frame total, no matter how large the array is. So for a simple linear task like summing an array, the iterative version is usually better in C.

#### B.1.4 Important Distinction: Recursive Idea vs Recursive Implementation

Some algorithms are easiest to understand recursively, but that does not always mean recursion is the best implementation. For example, this recursive definition is simple:

```text
sum(arr) = first element + sum(rest of array)
```

But the loop implementation is more efficient in C.

On the other hand, for tree traversal, recursion often stays natural and practical because each recursive call follows the shape of the tree. So the question is not only "is recursion good or bad?" but  also "does recursion make this problem clearer, and is the recursion depth safe?".

#### B.1.5 Tail recursion

A tail-recursive function is one where the recursive call is the last operation.

Example:

```c
int sum_tail(int *arr, int length, int acc) {
    if (length == 0) {
        return acc;
    }

    return sum_tail(arr + 1, length - 1, acc + arr[0]);
}
```

The recursive call is the final action. Some languages or compilers can optimize tail recursion so it behaves like a loop. This is called tail-call optimization. But in C, we should not rely on this optimization always happening. If stack usage matters, write an explicit loop instead.

---

### B.2 Exercise: Fibonacci Numbers - I

The goal is to implement the Fibonacci recurrence directly as a sequential recursive function:

```text
F(n) = F(n - 1) + F(n - 2)
```

We also need base cases so the recursion eventually stops.

The usual definition is:

```text
F(0) = 0
F(1) = 1
```

So:

```text
F(2) = F(1) + F(0) = 1
F(3) = F(2) + F(1) = 2
F(4) = F(3) + F(2) = 3
```

> [!IMPORTANT]
> Refer to [`./Codes/fibonacci_i.c`](./Codes/fibonacci_i.c) for the solution to this exercise.

#### B.2.1 Explanations to Solution

For our base case `n == 0`, return `0`. And for another base case `n == 1`, return `1`.

```c
if (n == 0) {
    return 0;
}

if (n == 1) {
    return 1;
}
```

This can also be written more compactly:

```c
if (n <= 1) {
    return n;
}
```

Then for larger `n`, use the recurrence directly:

```c
// to calculate fib(n), calculate the previous two Fibonacci numbers first
return fib(n - 1) + fib(n - 2);
```

#### B.2.2 Experiments: Measuring Time

Run the default `fib(42)`:

```bash
./fib
```

Or run a different value:

```bash
./fib 10
```

Use the `time` program to measure the time cost:

```bash
time ./fib 42
```

Example shape:

```text
267914296
real 1.23
user 1.22
sys 0.00
```

The exact time depends on the machine. The important observation is that `fib(42)` is noticeably slow compared with smaller values like `fib(10)` or `fib(20)`.

> [!NOTE]
> For more explanations of those "times", please refer to the following discussion.
> <https://stackoverflow.com/questions/556405/what-do-real-user-and-sys-mean-in-the-output-of-time1>
>
> **Following are copied from the discussion on Stack Overflow.**
>
> > `Real` is wall clock time - time from start to finish of the call. This is all elapsed time including time slices used by other processes and time the process spends blocked (for example if it is waiting for I/O to complete).
>
> > `User` is the amount of CPU time spent in user-mode code (outside the kernel) within the process. This is only actual CPU time used in executing the process. Other processes and time the process spends blocked do not count towards this figure.
>
> > `Sys` is the amount of CPU time spent in the kernel within the process. This means executing CPU time spent in system calls within the kernel, as opposed to library code, which is still running in user-space. Like 'user', this is only CPU time used by the process. See below for a brief description of kernel mode (also known as 'supervisor' mode) and the system call mechanism.
>
> > Keep in mind that `real` represents actual elapsed time, while user and sys values represent CPU execution time. As a result, on a multicore system, the `user` and/or `sys` time (as well as their sum) can actually exceed the `real` time.

The direct recursive Fibonacci function repeats a lot of work.

For example:

```text
fib(5)
= fib(4) + fib(3)
```

But:

```text
fib(4)
= fib(3) + fib(2)
```

So `fib(3)` is calculated more than once.

The call tree grows quickly:

```text
fib(5)
├── fib(4)
│   ├── fib(3)
│   └── fib(2)
└── fib(3)
    ├── fib(2)
    └── fib(1)
```

For `fib(42)`, there are millions of recursive calls.

> [!NOTE]
> **For `fib(42) = 267914296`, the function is called `866988873 = 2 * fib(43) + 1` times in total.**
>
> In the naive recursive call tree, base cases are reached `433494437 = fib(43)` times, where `267914296 = fib(42)` times for `fib(1)` , and `165580141 = fib(41)` times for `fib(0)`.
>
> The total calls **before** recursion reaches the base case are `433494436 = fib(43) - 1`.

---

### B.3 Exercise: Fibonacci Numbers - II

Our goal is to make some optimizations by parallelise **only the top level** of the Fibonacci recurrence:

```text
F(n) = F(n - 1) + F(n - 2)
```

We use three pthreads conceptually:

```text
T1: creates/waits for the two worker threads and combines the result
T2: computes fib(n - 1)
T3: computes fib(n - 2)
```

The important detail is that `T2` and `T3` still compute their values sequentially. We are not creating new threads at every recursive call.

That is intentional. If every recursive Fibonacci call created new threads, the program would create an enormous number of threads and be much slower or crash.

> [!IMPORTANT]
> Refer to [`./Codes/fibonacci_ii.c`](./Codes/fibonacci_ii.c) for the solution to this exercise.
> You need to use the `-pthread` flag when compiling.

#### B.3.1 Step 1: Sequential Fibonacci Function

This is copied from the previous exercise:

```c
long fib(int n) {
    if (n <= 1) {
        return n;
    }

    return fib(n - 1) + fib(n - 2);
}
```

This function is slow for large `n`, but it gives each worker thread a large enough task to do.

#### B.3.2 Step 2: Worker Thread for a Fibonacci Subproblem

Each worker needs:

```text
input n
output result
```

So we use a small struct:

```c
typedef struct {
    int n;
    long result;
} fib_task_t;
```

The worker receives a pointer to this struct, computes the result, and stores it back:

```c
static void *fib_worker(void *arg) {
    fib_task_t *task = arg;

    task->result = fib(task->n);

    return NULL;
}
```

#### B.3.3 Step 3: T1 Combines the Results

The T1 thread creates two workers:

```text
T2 computes fib(n - 1)
T3 computes fib(n - 2)
```

Then T1 joins both workers and adds their results:

```text
F(n) = result_from_T2 + result_from_T3
```

#### B.3.4 Experiments: Measuring Optimization

Similar to the previous exercise, you also need to call `time ./fib` to measure the running time. Then, calculate the speedup by `speedup = sequential time (Exercise Fib I) / parallel time (Exercise Fib II)` (or reversely if you like). We will see the speedup is usually limited.

At first, it might look like we should get close to a 2× speedup because two worker threads run at the same time. But the work is not evenly split.

For `fib(42)`:

```text
T2 computes fib(41)
T3 computes fib(40)
```

`fib(41)` takes more time than `fib(40)`. So after `T3` finishes, it may sit idle while `T2` is still working.

The total sequential work is roughly `fib(41) + fib(40)`, while the parallel time is roughly `max(fib(41), fib(40)) + thread overhead`.

Since `fib(41)` dominates, the best possible speedup from this simple split is around:

```text
(fib(41) + fib(40)) / fib(41)
```

which is less than 2.

In practice, thread creation and joining also add overhead, so the speedup may be even smaller.

---

### B.4 Exercise: Fibonacci Numbers - III

This version treats Fibonacci as a task-dependency problem.

For `F(n) = F(n - 1) + F(n - 2)`, the current task depends on two smaller tasks:

```text
left task   computes F(n - 1)
right task  computes F(n - 2)
```

Instead of using `pthread_join()`, each child task uses a semaphore to tell its parent: "my result is ready". The parent waits with `sem_wait()` and the child finishes and wakes the parent with `sem_post()`.

> [!IMPORTANT]
> We have two versions of solutions for this exercise.
>
> Refer to [`./Codes/fibonacci_iii_naive.c`](./Codes/fibonacci_iii_naive.c) to understand the concept behind the solution with usages of semaphores, but do not run it with a large `n`.
>
> **Refer to [`./Codes/fibonacci_iii.c`](./Codes/fibonacci_iii.c) for the final solution to this exercise.**
> You need to use the `-pthread` flag when compiling.

#### B.4.1 Step 1: Sequential Fibonacci Helper

We still need a sequential version. The optimised version will fall back to this when too many threads already exist.

```c
long fib_seq(int n) {
    if (n <= 1) {
        return n;
    }

    return fib_seq(n - 1) + fib_seq(n - 2);
}
```

#### B.4.2 Step 2: Fully Threaded Version Using Semaphores

Each task needs:

```text
n        input
result   output
done     semaphore used to signal completion
```

So we use:

```c
typedef struct {
    int n;
    long result;
    sem_t done;
} FibTask;
```

The child computes the result, then posts the semaphore:

```c
task->result = fib(task->n);
sem_post(&task->done);
```

The parent creates two child threads and waits for both semaphores.

This exercise asks us to perform the equivalent of a join using semaphores. But if we create a normal joinable thread and never call `pthread_join()`, the thread's resources may not be cleaned up immediately. So this solution uses:

```c
pthread_detach(thread);
```

A detached thread cleans up its own thread resources when it finishes. The parent still waits for the result using `sem_wait(&task.done);` So the semaphore handles completion, and `pthread_detach()` handles thread-resource cleanup.

#### B.4.3 Maximum Number of Threads in the Naive Version

The naive version creates two new threads for almost every non-base Fibonacci call. That grows extremely quickly. The number of recursive calls for naive Fibonacci is approximately `2 * F(n + 1) - 1` If the root call is made by the main thread, then the number of child threads created is roughly `2 * F(n + 1) - 2`.

For `fib(42)`:

```text
F(43) = 433494437
```

So the number of child threads that could be created is roughly:

```text
2 * 433494437 - 2 = 866988872
```

That is impossible to run on a normal machine.

The exact maximum number of threads alive at one time depends on scheduling, because some threads may finish before others are created. But the important point is the number of threads is exponential, and this hurts performance badly.

Instead of speeding up, the program can become much slower because of:

```text
thread creation overhead
semaphore overhead
too many stack allocations
context switching
scheduler pressure
memory exhaustion
failure to create new threads
```

So the fully threaded version is mainly a learning exercise, not a good implementation.

#### B.4.4 Optimised Version: Limit the Number of Worker Threads

To make the program more realistic, we limit how many worker threads can exist at once.

Use a semaphore as a thread budget:

```c
sem_t thread_slots;
```

If the semaphore has available slots, we create worker threads.

If the semaphore has no slots left, we stop creating more threads and compute the current subproblem sequentially:

```c
fib_seq(n - 1) + fib_seq(n - 2)
```

This avoids creating an exponential number of threads. Instead, the semaphore `thread_slots` controls how many worker threads may exist at once.

If:

```c
#define MAX_WORKER_THREADS 8
```

then at most 8 worker threads can be running or waiting at the same time, plus the original calling thread.

Each time the program wants to create a worker thread, it first takes a slot:

```c
sem_trywait(&thread_slots);
```

When the worker finishes, it returns the slot:

```c
sem_post(&thread_slots);
```

The completion semaphore is separate:

```c
task->done
```

That semaphore tells the parent that a specific child result is ready.

So there are two semaphore roles:

```text
task->done      waits for one child result
thread_slots    limits the number of worker threads
```

#### B.4.5 Is this faster? A Real-World Optimization Needed

It may be faster than the fully threaded version because it avoids creating millions of threads.

But it is still not the best Fibonacci algorithm.

The main issue is that naive Fibonacci repeats work:

```text
fib(42)
= fib(41) + fib(40)

fib(41)
= fib(40) + fib(39)
```

`fib(40)` gets computed more than once.

Threading does not fix repeated computation. It only tries to do some of it in parallel.

For Fibonacci, the better algorithmic fix is usually:

```text
iteration
memoization
dynamic programming
```

A simple loop is much faster than threaded naive recursion. Use multi-threads with care.
