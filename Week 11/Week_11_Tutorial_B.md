## COMP2017 2026 S1 Week 11 Tutorial B

<table><tbody>
  <tr><td><b>Tutor</b></td><td>Hao Ren</td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au">hao.ren@sydney.edu.au</a></td></tr>
</tbody></table>

[TOC]

---

### B.1 Deadlocks

A deadlock happens when two or more threads are stuck waiting for each other forever. In a mutex-based program, this usually means each thread is holding one lock and waiting for another lock that will never become available.

The program usually does not crash. It just stops making progress:

```text
no more output
no visible progress
threads are blocked waiting for locks
the program appears to hang
```

This is different from a race condition. A race condition means the result depends on unsafe timing. A deadlock means the program cannot continue because the threads are waiting in a cycle.

```text
race condition -> wrong result due to unsafe interleaving
deadlock       -> no progress due to circular waiting
```

#### B.1.1 How A Simple Deadlock Happens

Suppose there are two mutexes:

```c
pthread_mutex_t M1;
pthread_mutex_t M2;
```

Thread 1 locks them in this order:

```c
pthread_mutex_lock(&M1);
pthread_mutex_lock(&M2);

/* do work */

pthread_mutex_unlock(&M2);
pthread_mutex_unlock(&M1);
```

Thread 2 locks them in the opposite order:

```c
pthread_mutex_lock(&M2);
pthread_mutex_lock(&M1);

/* do work */

pthread_mutex_unlock(&M1);
pthread_mutex_unlock(&M2);
```

The dangerous part is not the unlock order. The dangerous part is the inconsistent lock acquisition order:

```text
T1: M1 then M2
T2: M2 then M1
```

A possible interleaving is:

```text
T1 locks M1
T2 locks M2
T1 tries to lock M2, but T2 has it
T2 tries to lock M1, but T1 has it
```

Now the state is:

```text
T1 holds M1 and waits for M2
T2 holds M2 and waits for M1
```

Neither thread can continue. T1 cannot unlock `M1` because it is blocked waiting for `M2`. T2 cannot unlock `M2` because it is blocked waiting for `M1`.

The wait cycle is:

```text
T1 -> M2 -> T2 -> M1 -> T1
```

Because the wait graph loops back to the starting thread, the program is deadlocked.

A common mistake is thinking the code is safe because both functions eventually contain unlock calls. The problem is that the threads may never reach those unlock lines.

#### B.1.2 The Four Conditions For Deadlock

A deadlock can happen when all four of these conditions are true.

```text
1. Mutual exclusion
   At least one resource can only be held by one thread at a time.
   A mutex is exactly this kind of resource.

2. Hold and wait
   A thread is holding one resource while waiting for another.

3. No preemption
   A lock cannot be forcibly taken from a thread.
   The thread that owns the lock must voluntarily unlock it.

4. Circular wait
   A cycle of waiting exists.
   For example, T1 waits for T2, and T2 waits for T1.
```

Breaking any one of these conditions prevents deadlock. In practice, the condition we most often break is circular wait, usually by enforcing a consistent lock order.

#### B.1.3 Diagnosing Deadlocks

When a threaded program hangs, the useful question is not just "which line is stuck?" The useful question is:

```text
Which locks exist?
Which thread holds each lock?
Which lock is each thread waiting for?
Do those waits form a cycle?
```

For the simple two-thread example, the diagnosis is:

```text
T1 holds M1, waits for M2
T2 holds M2, waits for M1
```

That immediately shows the cycle:

```text
T1 waits for T2
T2 waits for T1
```

Waiting alone is not deadlock. Circular waiting is deadlock.

For example, this is waiting but not necessarily deadlock:

```text
T1 holds M1
T2 waits for M1
T1 eventually unlocks M1
T2 continues
```

This is deadlock:

```text
T1 holds M1 and waits for M2
T2 holds M2 and waits for M1
```

because neither thread can reach the code that releases its lock.

Deadlocks may also be timing-dependent. A program might run correctly many times, then hang when the scheduler happens to interleave the threads in the wrong order. Adding `sleep()` can sometimes make a deadlock easier to reproduce, but it does not fix the bug.

#### B.1.4 Diagnosis Example: Three Mutexes

Suppose four threads use three mutexes `A`, `B`, and `C`:

```text
T1: lock(A), lock(B), lock(C), unlock(A), unlock(B), unlock(C)
T2: lock(A), lock(B), lock(C), unlock(C), unlock(B), unlock(A)
T3: lock(B), lock(C), unlock(B), unlock(C)
T4: lock(C), lock(A), unlock(A), unlock(C)
```

Focus on the lock order:

```text
T1: A -> B -> C
T2: A -> B -> C
T3:      B -> C
T4: C -> A
```

`T1`, `T2`, and `T3` are mostly consistent with the order:

```text
A before B before C
```

The dangerous thread is `T4`, because it locks:

```text
C before A
```

That creates a possible lock-order cycle:

```text
A -> B -> C -> A
```

Once a cycle exists in the lock ordering, there is a possible execution where the program deadlocks.

One possible two-thread deadlock is:

```text
1. T1 locks A
2. T4 locks C
3. T1 locks B
4. T1 tries to lock C
   - blocked, because T4 holds C

5. T4 tries to lock A
   - blocked, because T1 holds A
```

Now:

```text
T1 holds A and B, waits for C
T4 holds C, waits for A
```

The cycle is:

```text
T1 -> C -> T4 -> A -> T1
```

Another possible deadlock uses three threads:

```text
1. T2 locks A
2. T3 locks B
3. T4 locks C

4. T2 tries to lock B
   - blocked, because T3 holds B

5. T3 tries to lock C
   - blocked, because T4 holds C

6. T4 tries to lock A
   - blocked, because T2 holds A
```

Now:

```text
T2 holds A, waits for B
T3 holds B, waits for C
T4 holds C, waits for A
```

The cycle is:

```text
T2 -> B -> T3 -> C -> T4 -> A -> T2
```

So `T4` is the key source of the problem. It closes the cycle by acquiring `C` before `A`.

`T1` and `T2` alone should not deadlock with each other because they both lock in the same order:

```text
A -> B -> C
```

If `T1` gets `A` first, `T2` waits immediately on `A`. `T2` cannot hold `B` or `C` while waiting for `A`, so no cycle forms.

#### B.1.5 Preventing Deadlocks

The most common fix is to enforce a global lock order.

For the two-lock example, decide:

```text
Always lock M1 before M2.
```

Then both threads must follow that rule:

```c
pthread_mutex_lock(&M1);
pthread_mutex_lock(&M2);

/* critical section */

pthread_mutex_unlock(&M2);
pthread_mutex_unlock(&M1);
```

For the three-lock example, decide:

```text
Always lock A before B before C.
```

Then `T4` should not do this:

```text
lock(C), lock(A)
```

It should be rewritten as:

```text
lock(A), lock(C)
```

The exact unlock order is usually less important for avoiding the initial deadlock, but unlocking in reverse order is a good habit:

```text
lock A
lock B
lock C

unlock C
unlock B
unlock A
```

The main rule is:

```text
all threads must acquire shared locks in a consistent order
```

A second prevention strategy is to avoid holding one lock while waiting for another. Sometimes the code can be restructured:

```c
pthread_mutex_lock(&m1);
/* update only data protected by m1 */
pthread_mutex_unlock(&m1);

pthread_mutex_lock(&m2);
/* update only data protected by m2 */
pthread_mutex_unlock(&m2);
```

This only works if the operation does not need both resources protected at the same time. If both locks are genuinely needed together, use a consistent lock order.

Another strategy is to avoid slow or blocking operations while holding a lock. This is risky:

```c
pthread_mutex_lock(&lock);

sleep(3);
read(fd, buf, sizeof buf);
pthread_join(worker, NULL);

pthread_mutex_unlock(&lock);
```

If another thread needs `lock` to finish, then `pthread_join()` while holding the lock can deadlock:

```text
main thread holds lock and waits for worker
worker needs lock before it can finish
```

A better habit is:

```text
lock late
unlock early
```

Only protect the shared data access, not unrelated slow work.

`pthread_mutex_trylock()` can also help in some designs. It attempts to lock a mutex without blocking forever:

```c
pthread_mutex_lock(&M1);

if (pthread_mutex_trylock(&M2) != 0) {
    pthread_mutex_unlock(&M1);
    return NULL;
}

/* got both locks */

pthread_mutex_unlock(&M2);
pthread_mutex_unlock(&M1);
```

This can break the “hold and wait forever” pattern because the thread releases `M1` if it cannot get `M2`. However, `trylock` is not automatically simpler. Bad retry loops can cause livelock, where threads keep reacting to each other but still do not make useful progress.

Sometimes the best solution is to use fewer locks. If two pieces of shared state are almost always accessed together, one larger mutex may be safer:

```c
pthread_mutex_t book_lock;
```

instead of:

```c
pthread_mutex_t bids_lock;
pthread_mutex_t asks_lock;
```

The tradeoff is:

```text
one larger lock    simpler, less deadlock risk, less parallelism
many smaller locks more parallelism, more locking complexity
```

For beginner code, correctness is more important than maximum parallelism.

#### B.1.6 Common Deadlock Patterns

A common pattern is taking the same locks in different orders:

```c
/* Thread A */
pthread_mutex_lock(&a);
pthread_mutex_lock(&b);

/* Thread B */
pthread_mutex_lock(&b);
pthread_mutex_lock(&a);
```

The fix is to choose one order and use it everywhere.

Another common pattern is returning without unlocking:

```c
pthread_mutex_lock(&lock);

if (error) {
    return NULL;
}

pthread_mutex_unlock(&lock);
```

If `error` happens, the mutex stays locked forever. Every path that leaves a critical section must unlock first:

```c
pthread_mutex_lock(&lock);

if (error) {
    pthread_mutex_unlock(&lock);
    return NULL;
}

pthread_mutex_unlock(&lock);
```

A thread can also deadlock with itself:

```c
pthread_mutex_lock(&lock);
pthread_mutex_lock(&lock);
```

With a normal mutex, the second lock may block forever because the thread is waiting for a mutex it already owns. Recursive mutexes exist, but they should not be used just to hide unclear locking structure.

Another dangerous pattern is calling unknown code while holding a lock:

```c
pthread_mutex_lock(&lock);
some_function();
pthread_mutex_unlock(&lock);
```

If `some_function()` tries to take the same lock, or waits for another thread that needs the lock, the program may deadlock. Holding locks across function calls is not always wrong, but it requires knowing what the called function does.

#### B.1.7 Short Summary

A deadlock happens when threads are stuck waiting for resources held by each other.

The classic shape is:

```text
T1 holds M1 and waits for M2
T2 holds M2 and waits for M1
```

The four deadlock conditions are:

```text
mutual exclusion
hold and wait
no preemption
circular wait
```

The most important practical fix is consistent lock ordering:

```text
all threads acquire shared locks in the same order
```

To diagnose a deadlock, identify which thread holds which lock, which lock each thread is waiting for, and whether those waits form a cycle.

Keep critical sections small, avoid blocking while holding locks, unlock on every exit path, and be careful when using multiple mutexes.

---

### B.2 

---

### B.1 

---

### B.1 

---

### B.1 

---

### B.1 

