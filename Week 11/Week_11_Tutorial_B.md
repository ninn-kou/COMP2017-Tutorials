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

This can break the "hold and wait forever" pattern because the thread releases `M1` if it cannot get `M2`. However, `trylock` is not automatically simpler. Bad retry loops can cause livelock, where threads keep reacting to each other but still do not make useful progress.

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

### B.2 Exercise: Dining Philosophers

Each philosopher is a thread. Each chopstick is a mutex.

A philosopher can only eat after locking both adjacent chopsticks:

```text
philosopher i needs chopstick i and chopstick (i + 1) % THINKERS
```

The main problem is deadlock. If every philosopher picks up one chopstick and waits for the next one, everyone can get stuck forever. So our goals are:
1. allow each philosopher to acquire two adjacent chopsticks
2. avoid deadlock
3. release both chopsticks after eating

> [!IMPORTANT]
> Refer to [`phils.c`](./Codes/phils.c) for the solution to this exercise.
> You need to use the `-pthread` flag when compiling.
>
> The original scaffold uses `while (true)`, which is fine for an endless simulation. However, if the loop is infinite, `pthread_join()` never returns and the mutex destroy code is never reached.
>
> My solution uses a small fixed number of meals so the program is easy to test. To make it endless, replace the `for` loop in `dine()` with `while (true)`.

#### B.2.1 Design Choice

The safest simple strategy is to impose a global lock order.

Instead of every philosopher locking left then right, each philosopher locks the lower-numbered chopstick first, then the higher-numbered chopstick.

For example:

```text
philosopher 0 needs chopsticks 0 and 1 -> lock 0 then 1
philosopher 1 needs chopsticks 1 and 2 -> lock 1 then 2
philosopher 4 needs chopsticks 4 and 0 -> lock 0 then 4
```

This avoids circular wait. The circular wait is what causes deadlock, so breaking the lock cycle prevents the deadlock.

#### B.2.2 Step 1: Work Out the Two Chopsticks

For philosopher `id`:

```c
unsigned left = id;
unsigned right = (id + 1) % THINKERS;
```

The modulo matters for the last philosopher.

If `THINKERS == 5`, philosopher `4` needs:

```text
chopstick 4 and chopstick 0
```

because the table is circular.

#### B.2.3 Step 2: Lock in a Consistent Order

Use:

```c
unsigned first = left < right ? left : right;
unsigned second = left < right ? right : left;
```

Then:

```c
pthread_mutex_lock(&chopsticks[first]);
pthread_mutex_lock(&chopsticks[second]);
```

Now all philosophers obey the same global lock ordering.

#### B.2.4 Step 3: Eat, Then Unlock

After eating, unlock both chopsticks:

```c
pthread_mutex_unlock(&chopsticks[second]);
pthread_mutex_unlock(&chopsticks[first]);
```

Unlocking in reverse order is a common habit for nested locks.

#### B.2.5 Notes

It is dangerous:

```c
pthread_mutex_lock(&chopsticks[left]);
pthread_mutex_lock(&chopsticks[right]);
```

If every philosopher does this at the same time, this can happen:

```text
philosopher 0 holds chopstick 0, waits for 1
philosopher 1 holds chopstick 1, waits for 2
philosopher 2 holds chopstick 2, waits for 3
philosopher 3 holds chopstick 3, waits for 4
philosopher 4 holds chopstick 4, waits for 0
```

Now everyone is waiting for someone else.

That is circular wait.

The fix is to avoid letting the circular wait form. By always locking the lower-numbered mutex first, every philosopher follows the same global ordering:

```text
0 before 1 before 2 before 3 before 4
```

The last philosopher needs chopsticks `4` and `0`, but still locks `0` first, then `4`.

So the cycle is broken.

> [!NOTE]
> A common mistake is forgetting the modulo:
>
> ```c
> unsigned right = id + 1;
> ```
>
> For the last philosopher, this gives an invalid chopstick index. Use:
>
> ```c
> unsigned right = (id + 1) % THINKERS;
> ```
>
> Another common mistake is locking left then right for every philosopher. That can deadlock.
>
> Also, be careful when passing thread IDs. This is good:
>
> ```c
> args[i] = i;
> pthread_create(&thinkers[i], NULL, dine, &args[i]);
> ```
>
> This is unsafe:
>
> ```c
> pthread_create(&thinkers[i], NULL, dine, &i);
> ```
>
> because every thread receives the address of the same loop variable.
>
> Finally, `pthread` functions return error numbers directly. They do not always set `errno`, so `strerror(err)` is usually clearer than `perror()` for pthread errors.

---

### B.3 Exercise: DNS Cache I

The DNS cache stores mappings like:

```text
google.com  -> 142.250.70.46
```

The provided test program creates several getter threads and one putter thread. The getter threads repeatedly call `cache_get()` until a domain appears in the cache, and the putter thread slowly inserts entries using `cache_put()`. The tester also calls `free(ip)` after a successful `cache_get()`, so `cache_get()` must return a heap-allocated copy of the IP address, not a pointer into the cache itself.

To design our solution, we use one mutex inside the cache:

```c
pthread_mutex_t lock;
```

This mutex protects the whole cache array. The rule is simple that any thread reading or writing the cache must hold the mutex.

This means:

```text
cache_get locks before searching
cache_put locks before updating or inserting
cache_destroy locks while cleaning up state
```

For this first version, one mutex is completely fine. A more advanced DNS cache could use a hash table with several locks, but the scaffold uses a small fixed array, so one lock keeps the implementation clear.

> [!IMPORTANT]
> Refer to the folder [`dns_cache_I/*`](./Codes/dns_cache_I/) for codes.

```text
dns_cache_I/
├── dns_cache.h
├── dns_cache.c
└── thread_pool.c
```

Compile:

```bash
gcc -std=c11 -Wall -Wextra -pedantic -g thread_pool.c dns_cache.c -o thread_pool -pthread
```

#### B.3.1 Explanations

The shared state is:

```c
dns_cache_t cache;
```

Inside the cache, the shared data is:

```c
cache_entry_t entries[CACHE_SIZE];
int count;
```

Multiple getter threads may search the array while the putter thread inserts or updates entries. Without a mutex, one thread could read the cache while another thread is halfway through writing an entry.

So both operations lock the same mutex:

```c
pthread_mutex_lock(&c->lock);
```

and unlock when finished:

```c
pthread_mutex_unlock(&c->lock);
```

`cache_get()` duplicates the IP before returning:

```c
result = copy_string(c->entries[i].ip);
```

This matters because the caller does:

```c
free(ip);
```

If `cache_get()` returned the internal `entry->ip` array, the caller would try to free memory that was not allocated with `malloc()`, which would be invalid.

#### B.3.2 Common Mistakes

A common mistake is forgetting to include `<arpa/inet.h>`. The scaffold uses:

```c
INET_ADDRSTRLEN
```

so the header should include:

```c
#include <arpa/inet.h>
```

Another mistake is returning the internal cache storage:

```c
return c->entries[i].ip;
```

That is wrong here because the tester frees the returned pointer.

Also, do not skip locking in `cache_get()` just because it only reads. A read can still race with a writer thread updating or inserting an entry.

---

### B.4 Read-Write Locks

A **read-write lock** is useful when many threads need to read shared data, but only some threads need to modify it.

A normal mutex has one mode:

```text
one thread at a time
```

A read-write lock has two modes:

```text
many readers at the same time:    reads can safely happen together
one writer by itself:             writes must happen alone
```

This is useful for shared data structures that are read often but changed less often, such as caches, lookup tables, configuration data, or mostly-read linked lists.

A read operation only looks at shared data:

```text
searching
checking
printing
looking up a value
```

A write operation changes shared data:

```text
inserting
removing
updating
resizing
freeing nodes
```

So the rule is:

```text
only observe shared data  -> read lock
change shared data        -> write lock
```

```c
// The POSIX type is:
pthread_rwlock_t

// The main functions are:
pthread_rwlock_rdlock()
pthread_rwlock_wrlock()
pthread_rwlock_unlock()

// There are also functions for initialisation and cleanup:
pthread_rwlock_init()
pthread_rwlock_destroy()
```

A read-write lock can be dynamically initialised like this:

```c
pthread_rwlock_t lock;

pthread_rwlock_init(&lock, NULL);

/* use lock */

pthread_rwlock_destroy(&lock);
```

For a global or static read-write lock, static initialisation is also possible:

```c
static pthread_rwlock_t lock = PTHREAD_RWLOCK_INITIALIZER;
```

Compile pthread programs with:

```bash
gcc -std=c11 -Wall -Wextra -pedantic program.c -o program -pthread
```

> [!IMPORTANT]
> A read-write lock must be initialised before use.
>
> This is not safe:
>
> ```c
> pthread_rwlock_t lock;
> pthread_rwlock_rdlock(&lock);
> ```
>
> Use `PTHREAD_RWLOCK_INITIALIZER` or `pthread_rwlock_init()` first.

For read-only access, use a **read lock**:

```c
pthread_rwlock_rdlock(&lock);

/* read shared data */

pthread_rwlock_unlock(&lock);
```

Many threads may hold the read lock at the same time, as long as no writer holds the lock.

For example:

```c
pthread_rwlock_rdlock(&cache_lock);

char *ip = search_cache(domain);

pthread_rwlock_unlock(&cache_lock);
```

This means:

```text
I am only reading the cache.
Other readers may read at the same time.
A writer must wait until the readers are finished.
```

> [!NOTE]
> A read lock is still a lock.
>
> This is unsafe if another thread may modify the cache:
>
> ```c
> char *ip = search_cache(domain);
> ```
>
> Even though the operation only reads, it can still race with a writer. For example, one thread might be walking through a linked list while another thread removes a node.

For modifying access, use a **write lock**:

```c
pthread_rwlock_wrlock(&lock);

/* modify shared data */

pthread_rwlock_unlock(&lock);
```

Only one thread can hold the write lock. While a writer holds the lock:

```text
no other writer can enter
no reader can enter
```

For example:

```c
pthread_rwlock_wrlock(&cache_lock);

insert_or_update_cache(domain, ip);

pthread_rwlock_unlock(&cache_lock);
```

This means:

```text
I am changing the cache.
Nobody else may read or write the cache during this update.
```

> [!IMPORTANT]
> Do not modify shared data while holding only a read lock.
>
> This is wrong:
>
> ```c
> pthread_rwlock_rdlock(&lock);
> shared_value++;
> pthread_rwlock_unlock(&lock);
> ```
>
> A read lock allows other readers to enter. If this thread modifies the data, other readers may see inconsistent data.
>
> Use a write lock instead:
>
> ```c
> pthread_rwlock_wrlock(&lock);
> shared_value++;
> pthread_rwlock_unlock(&lock);
> ```

Both read locks and write locks are released with the same function:

```c
pthread_rwlock_unlock(&lock);
```

So the two basic shapes are:

```c
pthread_rwlock_rdlock(&lock);
/* read shared data */
pthread_rwlock_unlock(&lock);
```

and:

```c
pthread_rwlock_wrlock(&lock);
/* modify shared data */
pthread_rwlock_unlock(&lock);
```

> [!IMPORTANT]
> Refer to [`rwlock.c*`](./Codes/rwlock.c) for demonstration.
> Don't forget to compile with `-pthread` flag.

Read-write locks are not always better than mutexes. A mutex is simpler:

```text
only one thread enters at a time
```

A read-write lock allows more concurrency:

```text
many readers can enter together
writers enter alone
```

A mutex is often better when:

```text
the protected operation is very small
writes are frequent
the code should stay simple
```

A read-write lock is often better when:

```text
reads are common
writes are uncommon
read operations take long enough that parallel readers help
```

For example, a cache is a natural use case:

```text
cache_get()  uses a read lock
cache_put()  uses a write lock
```

Many threads can look up entries at the same time, but inserting or updating an entry must be exclusive.

> [!NOTE]
> Keep write-locked sections short.
>
> This works, but it blocks all readers and writers while sleeping:
>
> ```c
> pthread_rwlock_wrlock(&lock);
>
> sleep(5);
> update_shared_data();
>
> pthread_rwlock_unlock(&lock);
> ```
>
> Better:
>
> ```c
> sleep(5);
>
> pthread_rwlock_wrlock(&lock);
> update_shared_data();
> pthread_rwlock_unlock(&lock);
> ```
>
> The write lock should protect the shared update, not unrelated work.

The order in which waiting readers and writers are allowed to continue depends on the lock’s policy and implementation. Do not write code that depends on readers or writers waking in a specific order.

> [!NOTE]
>
> ```text
> use a mutex first if the design is simple
> use a read-write lock when the data is clearly read-heavy
> ```
>
> Read-write locks have overhead. If the shared operation is tiny or writes are common, a normal mutex may be faster and easier to reason about.

Only destroy the lock after all threads have finished using it:

```c
pthread_rwlock_destroy(&lock);
```

For example, this is the right place:

```c
pthread_join(reader_thread, NULL);
pthread_join(writer_thread, NULL);

pthread_rwlock_destroy(&lock);
```

Do not destroy a read-write lock while another thread might still read-lock, write-lock, unlock, or wait on it.

---

### B.5 Exercise: DNS Cache II

> [!IMPORTANT]
> Refer to the folder [`dns_cache_II/*`](./Codes/dns_cache_II/) for codes.
> Use `-pthread` flag for multi-threads program.
> Since this is a GNU extension, compile with: `-D_GNU_SOURCE` as well.

```text
dns_cache_II/
├── dns_cache.h
├── dns_cache.c
└── thread_pool.c
```

Compile:

```bash
gcc -D_GNU_SOURCE -std=c11 -Wall -Wextra -pedantic -g thread_pool.c dns_cache.c -o thread_pool -pthread
```

In DNS Cache I, the cache used one mutex to protect the whole linked list. That was correct, but it also meant only one thread could use the cache at a time.

For example, even if five getter threads were only searching the cache, they still had to take turns:

```text
getter 1 searches
getter 2 waits
getter 3 waits
getter 4 waits
getter 5 waits
```

DNS Cache II improves this by replacing the mutex with a read-write lock:

```c
pthread_rwlock_t lock;
```

A read-write lock has two access modes:

```text
read lock   many readers may hold the lock together
write lock  one writer holds the lock by itself
```

This is useful for a cache because lookups are usually read-only, but insertions and updates modify the linked list.

The main rule for this exercise is:

```text
cache_get()      uses a read lock
cache_put()      uses a write lock
cache_destroy()  uses a write lock
```

`cache_get()` only searches the linked list, so it can share the lock with other getters:

```c
pthread_rwlock_rdlock(&cache->lock);

/* search the linked list */

pthread_rwlock_unlock(&cache->lock);
```

This allows multiple getter threads to search the cache at the same time, as long as no writer is modifying the cache.

> [!IMPORTANT]
> A read lock is still a lock.
>
> This is unsafe if another thread might update the cache at the same time:
>
> ```c
> search_cache(domain);
> ```
>
> Even though the getter only reads, it may be walking through the linked list while another thread inserts, updates, or frees a node. The getter still needs a read lock.

`cache_put()` changes the cache, so it must use a write lock:

```c
pthread_rwlock_wrlock(&cache->lock);

/* update an existing entry or insert a new entry */

pthread_rwlock_unlock(&cache->lock);
```

A write lock is exclusive. While a writer holds the lock:

```text
no other writer can enter
no reader can enter
```

This matters because `cache_put()` may change node contents or change linked-list pointers.

For example, it may replace an existing IP address:

```c
free(curr->ip);
curr->ip = duplicate_string(ip);
```

or insert a new node:

```c
new_node->next = cache->head;
cache->head = new_node;
```

Both operations modify shared data, so both must happen while holding the write lock.

> [!NOTE]
> Do not use a read lock in `cache_put()`.
>
> This is wrong:
>
> ```c
> pthread_rwlock_rdlock(&cache->lock);
> ```
>
> `cache_put()` modifies the cache, so it needs:
>
> ```c
> pthread_rwlock_wrlock(&cache->lock);
> ```

`cache_destroy()` should also use the write lock because it frees the linked list:

```c
pthread_rwlock_wrlock(&cache->lock);

/* free all cache nodes */

pthread_rwlock_unlock(&cache->lock);
```

Freeing nodes is a modification. No getter should be traversing the list while the destroy function is freeing it.

The cache should initialise and destroy the read-write lock as part of the cache lifecycle:

```c
int cache_init(dns_cache_t *cache) {
    cache->head = NULL;
    return pthread_rwlock_init(&cache->lock, NULL);
}
```

and later:

```c
pthread_rwlock_destroy(&cache->lock);
```

Only destroy the lock after all threads have finished using the cache.

The Part I linked-list cache should keep the full struct definition visible in the header if the tester creates a real cache object:

```c
static dns_cache_t cache;
```

For this to compile, the compiler must know the full size of `dns_cache_t`. So the header should contain the actual struct definition, not only:

```c
typedef struct dns_cache dns_cache_t;
```

A linked-list version may look like:

```c
typedef struct dns_entry {
    char *domain;
    char *ip;
    struct dns_entry *next;
} dns_entry_t;

typedef struct dns_cache {
    dns_entry_t *head;
    pthread_rwlock_t lock;
} dns_cache_t;
```

> [!NOTE]
> Keep the `.h` and `.c` files consistent.
>
> Do not mix the linked-list Part I design with an array-based scaffold. The linked-list version uses:
>
> ```c
> dns_entry_t *head;
> ```
>
> An array-based version may use:
>
> ```c
> entries[CACHE_SIZE]
> count
> ```
>
> Pick one design and use it consistently.

One important ownership rule is that `cache_get()` must return a heap-allocated copy of the IP address.

The getter thread frees the returned string:

```c
free(ip);
```

So `cache_get()` must not return the internal pointer stored inside the cache.

This is wrong:

```c
return curr->ip;
```

That gives the caller a pointer owned by the cache. If the caller frees it, the cache now contains a dangling pointer.

Instead, return a copy:

```c
char *ip_copy = duplicate_string(curr->ip);
return ip_copy;
```

The rule is:

```text
cache owns the strings stored inside the linked list
caller owns the string returned by cache_get()
```

So a successful `cache_get()` should follow this shape:

```c
char *cache_get(dns_cache_t *cache, const char *domain) {
    pthread_rwlock_rdlock(&cache->lock);

    dns_entry_t *curr = cache->head;

    while (curr != NULL) {
        if (strcmp(curr->domain, domain) == 0) {
            char *ip_copy = duplicate_string(curr->ip);

            pthread_rwlock_unlock(&cache->lock);
            return ip_copy;
        }

        curr = curr->next;
    }

    pthread_rwlock_unlock(&cache->lock);
    return NULL;
}
```

Notice that the copy is made before unlocking. This ensures the getter copies the IP while the cache entry is still protected.

A successful `cache_put()` should follow this shape:

```c
int cache_put(dns_cache_t *cache, const char *domain, const char *ip) {
    pthread_rwlock_wrlock(&cache->lock);

    dns_entry_t *curr = cache->head;

    while (curr != NULL) {
        if (strcmp(curr->domain, domain) == 0) {
            char *new_ip = duplicate_string(ip);

            if (new_ip == NULL) {
                pthread_rwlock_unlock(&cache->lock);
                return -1;
            }

            free(curr->ip);
            curr->ip = new_ip;

            pthread_rwlock_unlock(&cache->lock);
            return 0;
        }

        curr = curr->next;
    }

    dns_entry_t *new_entry = malloc(sizeof *new_entry);

    if (new_entry == NULL) {
        pthread_rwlock_unlock(&cache->lock);
        return -1;
    }

    new_entry->domain = duplicate_string(domain);
    new_entry->ip = duplicate_string(ip);

    if (new_entry->domain == NULL || new_entry->ip == NULL) {
        free(new_entry->domain);
        free(new_entry->ip);
        free(new_entry);

        pthread_rwlock_unlock(&cache->lock);
        return -1;
    }

    new_entry->next = cache->head;
    cache->head = new_entry;

    pthread_rwlock_unlock(&cache->lock);
    return 0;
}
```

This code uses the write lock because it may update an existing node or insert a new one.

> [!IMPORTANT]
> Every successful lock needs a matching unlock.
>
> This is especially important when returning early after an error or successful lookup. Before returning from a locked section, unlock the read-write lock.

This exercise says writes are more important than reads. That is reasonable for this cache because delayed writes cause repeated misses.

For example:

```text
getter wants example.com
cache does not contain example.com yet
getter retries later
putter eventually inserts example.com
getter can now succeed
```

If the putter is delayed for too long, all getter threads waiting for that domain keep missing.

On GNU/Linux, writer preference can be requested with a non-portable read-write-lock attribute:

```c
pthread_rwlockattr_setkind_np(
    &attr,
    PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP
);
```

A cache initialisation function using this attribute may look like:

```c
int cache_init(dns_cache_t *cache) {
    pthread_rwlockattr_t attr;

    cache->head = NULL;

    int err = pthread_rwlockattr_init(&attr);

    if (err != 0) {
        return err;
    }

    err = pthread_rwlockattr_setkind_np(
        &attr,
        PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP
    );

    if (err != 0) {
        pthread_rwlockattr_destroy(&attr);
        return err;
    }

    err = pthread_rwlock_init(&cache->lock, &attr);

    pthread_rwlockattr_destroy(&attr);
    return err;
}
```

If the GNU writer-preference option is not available, use the normal POSIX initialisation instead:

```c
int cache_init(dns_cache_t *cache) {
    cache->head = NULL;
    return pthread_rwlock_init(&cache->lock, NULL);
}
```

This still gives the main read-write-lock behaviour:

```text
many getters can read together
putters still update exclusively
```

It just does not explicitly request writer preference.

> [!NOTE]
> The `_np` / `_NP` names mean non-portable.
>
> They are useful on GNU/Linux, but they are not standard POSIX. If portability matters, use:
>
> ```c
> pthread_rwlock_init(&cache->lock, NULL);
> ```

A read-write lock is useful for this exercise because the workload is read-heavy:

```text
many cache_get() calls
fewer cache_put() calls
```

It is less useful when:

```text
most operations are writes
there is only one thread
the locked work is extremely small
there is little contention
```

If almost every operation is `cache_put()`, then the write lock is exclusive most of the time. In that case, the read-write lock behaves like a more complicated mutex.

The basic rule is:

```text
many reads, few writes  -> read-write lock can help
many writes             -> mutex may be simpler
```

For DNS Cache II, the intended design is:

```text
cache_get()      read lock, returns a copied IP string
cache_put()      write lock, inserts or updates entries
cache_destroy()  write lock, frees the linked list
```

The main improvement over DNS Cache I is that multiple getter threads can search the cache at the same time, while writers still have exclusive access when updating the linked list.
