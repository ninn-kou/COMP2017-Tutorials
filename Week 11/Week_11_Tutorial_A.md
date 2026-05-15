## COMP2017 2026 S1 Week 11 Tutorial A

<table><tbody>
  <tr><td><b>Tutor</b></td><td>Hao Ren</td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au">hao.ren@sydney.edu.au</a></td></tr>
</tbody></table>

[TOC]

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

### A.1 

---

### A.1 

---

### A.1 

---

### A.1 

---

### A.1 
