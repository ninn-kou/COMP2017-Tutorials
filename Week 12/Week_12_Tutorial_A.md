## COMP2017 2026 S1 Week 12 Tutorial A

<table><tbody>
  <tr><td><b>Tutor</b></td><td>Hao Ren</td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au">hao.ren@sydney.edu.au</a></td></tr>
</tbody></table>

- [COMP2017 2026 S1 Week 12 Tutorial A](#comp2017-2026-s1-week-12-tutorial-a)
  - [A.1 Synchronisation](#a1-synchronisation)
  - [A.2 Semaphores](#a2-semaphores)
    - [A.2.1 Let's Design Semaphores Together: WcDonald's](#a21-lets-design-semaphores-together-wcdonalds)
    - [A.2.2 Semaphore Usage: Threads vs. Processes](#a22-semaphore-usage-threads-vs-processes)
    - [A.2.3 Using POSIX Semaphores Between Threads](#a23-using-posix-semaphores-between-threads)
    - [A.2.4 Using POSIX Semaphores Between Processes](#a24-using-posix-semaphores-between-processes)
    - [A.2.5 Thread Semaphores vs. Process Semaphores](#a25-thread-semaphores-vs-process-semaphores)
    - [A.2.6 Important Notes](#a26-important-notes)
    - [A.2.7 Binary Semaphore Is a Mutex](#a27-binary-semaphore-is-a-mutex)
  - [A.3 Exercise: Producer-Consumer](#a3-exercise-producer-consumer)
    - [A.3.1 Running the Demo](#a31-running-the-demo)
    - [A.3.2 Explanation for Notes](#a32-explanation-for-notes)
    - [A.3.3 Why Both Mutex and Semaphore Are Needed?](#a33-why-both-mutex-and-semaphore-are-needed)
    - [A.3.4 Shutdown Using Sentinel Values](#a34-shutdown-using-sentinel-values)
    - [A.3.5 Common Mistakes](#a35-common-mistakes)
  - [A.4 Exercise: Dining Semaphores](#a4-exercise-dining-semaphores)
    - [A.4.1 Step 1: Initialise the Semaphore](#a41-step-1-initialise-the-semaphore)
    - [A.4.2 Step 2: Use the Semaphore in `dine()`](#a42-step-2-use-the-semaphore-in-dine)
    - [A.4.3 Explanation for Notes](#a43-explanation-for-notes)
  - [A.5 Exercise: The Tortoise and the Hare](#a5-exercise-the-tortoise-and-the-hare)
    - [A.5.1 Step 1: Add a Semaphore](#a51-step-1-add-a-semaphore)
    - [A.5.2 Step 2: Tortoise Posts When Finished](#a52-step-2-tortoise-posts-when-finished)
    - [A.5.3 Step 3: Hare Waits before Finishing](#a53-step-3-hare-waits-before-finishing)
  - [A.6 Condition Variables](#a6-condition-variables)
    - [A.6.1 POSIX Condition Variable Functions](#a61-posix-condition-variable-functions)
    - [A.6.2 The Standard Waiting Pattern](#a62-the-standard-waiting-pattern)
    - [A.6.3 The Standard Signalling Pattern](#a63-the-standard-signalling-pattern)
    - [A.6.4 Difference between Waiting on a Mutex and Waiting on a Condition Variable](#a64-difference-between-waiting-on-a-mutex-and-waiting-on-a-condition-variable)
    - [A.6.5 Producer-Consumer Intuition](#a65-producer-consumer-intuition)

---

### A.1 Synchronisation

Shared memory allows two processes to access the same memory, but it does **not** control timing. For example, a reader might try to read before the writer has written anything:

```text
Writer: writes data into shared memory
Reader: reads data from shared memory
```

Without synchronization, this can happen:

```text
Reader reads too early
Reader gets old or invalid data
```

A semaphore can fix this by making the reader wait until the writer says the data is ready.

---

### A.2 Semaphores

A **semaphore** is a synchronization tool used to control access to shared resources or to signal between processes/threads.

Think of a semaphore as a **counter**.

```text
P: sem_wait():
    If the counter is greater than 0, decrease it and continue.
    If the counter is 0, block and wait.

V: sem_post():
    Increase the counter.
    Wake up one waiting process/thread if there is one.
```

You might notice we use two "strange" character `P` (Wait) and `V` (Signal). Here is the resource about the naming: [Origins of P( ) and V( )](https://cs.nyu.edu/~yap/classes/os/resources/origin_of_PV.html).

#### A.2.1 Let's Design Semaphores Together: WcDonald's

The number of shared resources is very limited. Before a thread can access a shared resource, it must check whether the resource is currently being used by another thread. If the resource is unavailable, the thread must wait until it becomes available again.

To represent this, we can use a global variable to indicate whether the resource is available.

```c
// The number of available resources.
int S = 1;
```

When a thread accesses the shared resource, it needs to mark the resource as unavailable. Once the thread is finished, it also needs to mark the resource as available again.

Therefore, every thread should call `P()` to request access to the resource and `V()` to release the resource.

Let's design these two functions together.

```c
// Request the resource.
void P(void) {
    // If S <= 0, there are no available resources.
    // Wait until the resource becomes available.
    while (S <= 0);

    // S > 0, so the resource is available.
    // Mark the resource as in use.
    S--;
}

// Release the resource.
void V(void) {
    // Mark the resource as available again.
    S++;
}
```

However, there are still some problems with our design. Let's look at the following example.

In WcDonald's, suppose we have two types of shared resources: fries and burgers. Each type of resource may have multiple available instances. Customers who request these resources can be treated as threads.

<img src="../assets/img/semaphore_busy_wait_1.png" alt="Semaphore Busy Wait 1" width="75%">

Now, Customer `A` requests the last available order of fries. Customer `B` also wants to order fries. However, the cashier does not know how many orders of fries are still in stock. As a result, the cashier goes to the kitchen and repeatedly asks, "Are the fries ready yet?" for 10 minutes until another order of fries becomes available. Only then can the cashier bring the fries back to Customer `B`.

Now let's look at Customer `C`. Customer `C` only wants a burger, and we know that burgers are still in stock. However, Customer `C` still has to wait because the cashier is busy waiting for fries for Customer `B`.

<img src="../assets/img/semaphore_busy_wait_2.png" alt="Semaphore Busy Wait 2" width="75%">

<img src="../assets/img/semaphore_busy_wait_3.png" alt="Semaphore Busy Wait 3" width="75%">

<img src="../assets/img/semaphore_busy_wait_4.png" alt="Semaphore Busy Wait 4" width="75%">

This is extremely inefficient. The problem is that one waiting thread can block everyone behind it.

How can we improve this design?

Instead of making the cashier repeatedly check whether the fries are ready, what if the cashier takes the order first and gives the customer a number ticket? If there are currently no fries in stock, the customer can wait in a queue. Once the fries are ready, the cashier can notify the next customer in the queue.

<img src="../assets/img/semaphore_counting_1.png" alt="Counting Semaphores 1" width="75%">

Now we need to modify our data structure. Each semaphore should maintain both a counter and a waiting queue. The counter tells us how many resources are available, while the waiting queue stores the threads that are currently blocked.

Our `P()` and `V()` functions could look like this:

```c
// Pseudocode
typedef struct {
    int value;   // Number of available resources.
    Process/Thread *L;
} Semaphore;

// Request a resource.
void P(Semaphore *S) {
    S->value--;

    if (S->value < 0) {
        // No resource is available.
        // Block the current thread and put it into the waiting queue.
        block(S->L);
    }
}

// Release a resource.
void V(Semaphore *S) {
    S->value++;

    if (S->value <= 0) {
        // At least one thread is waiting.
        // Wake up all thread and add 1 to their value. Then, block those whose value still less than 0.
        wakeup(S->L);
    }
}
```

<img src="../assets/img/semaphore_counting_2.png" alt="Counting Semaphores 2" width="75%">

Now every customer has a known position in the waiting queue.

<img src="../assets/img/semaphore_counting_3.png" alt="Counting Semaphores 3" width="75%">

<img src="../assets/img/semaphore_counting_4.png" alt="Counting Semaphores 4" width="75%">

<img src="../assets/img/semaphore_counting_5.png" alt="Counting Semaphores 5" width="75%">

Once a resource becomes available because another thread or process releases it, the semaphore wakes up all waiting thread and add `1` (or more if multiple resources are released) to their values. Then, every thread will check if the current one is their order (if `value < 0` means no). Even no, they will at least know their updated position in the queue.

The important improvement is that waiting threads no longer need to repeatedly check whether the resource is available. Instead, they are blocked and placed into a queue. When the resource becomes available, the semaphore wakes up the appropriate waiting thread.

<img src="../assets/img/semaphore_counting_6.png" alt="Counting Semaphores 6" width="75%">

However, there are still one thing could be improved. Waking all waiting threads would create unnecessary contention and may cause another busy-wait-like problem.

A standard counting semaphore usually wakes one waiting thread per released resource. If multiple resources are released, multiple waiting threads may be woken up, one for each newly available resource. By using the data structure queue, we could figure out this problem.

```c
typedef struct {
    int value;   // Number of available resources.
    Queue *L;    // Queue of blocked threads.
} Semaphore;

// Request a resource.
void P(Semaphore *S) {
    S->value--;

    if (S->value < 0) {
        // No resource is available.
        // Block the current thread and put it into the waiting queue.
        block(S->L);
    }
}

// Release a resource.
void V(Semaphore *S) {
    S->value++;

    if (S->value <= 0) {
        // At least one thread is waiting.
        // Wake up one thread from the waiting queue.
        wakeup(S->L);
    }
}
```

Technical note: this is a simplified first design. In a real semaphore implementation, checking S and updating S must be done **atomically**; otherwise, multiple threads may still enter the critical section at the same time.

🎉 Congratulations! You have designed the semaphore mechanism! I am proud of you.

Below is a redesigned continuation starting from **A.2.2**, with the usage of semaphores clearly split into **threads** and **processes**.

#### A.2.2 Semaphore Usage: Threads vs. Processes

So far, we have designed a semaphore conceptually. A semaphore contains two important parts:

```c
typedef struct {
    int value;   // Number of available resources.
    Queue *L;    // Queue of blocked threads or processes.
} Semaphore;
```

The operations are:

```c
P(S);  // Request a resource. Also called wait/down.
V(S);  // Release a resource. Also called signal/up.
```

However, when we actually use semaphores in a program, we need to be careful about one important question: **Are we synchronizing threads or processes?**

This matters because **threads and processes share memory differently**. Threads in the same process share the same address space. This means they can directly share the same semaphore variable. Processes, on the other hand, usually have separate address spaces. A normal global variable in one process is not automatically shared with another process. Therefore, if we want to use a semaphore between processes, the semaphore must be placed in shared memory or created as a named semaphore.

#### A.2.3 Using POSIX Semaphores Between Threads

Threads inside the same process share memory. Therefore, if we create a semaphore as a global variable, all threads in that process can access the same semaphore.

For example, suppose WcDonald's has only `3` available tables. Multiple customers may enter the restaurant, but at most `3` customers can use a table at the same time. We can represent the number of available tables with a semaphore.

```c
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_CUSTOMERS 6
#define NUM_TABLES 3

sem_t tables;
```

Here, `tables` is a shared semaphore. Since all threads belong to the same process, every thread can access this same variable.

Before a customer uses a table, the customer must call `sem_wait()`.

```c
sem_wait(&tables);
```

This is equivalent to our previous `P()` operation.

If there is an available table, the semaphore value decreases by `1`, and the customer continues.

If there is no available table, the customer thread is blocked and placed into the semaphore's waiting queue.

After the customer finishes using the table, the customer calls `sem_post()`.

```c
sem_post(&tables);
```

This is equivalent to our previous `V()` operation.

It increases the semaphore value by `1`. If another thread is waiting, one waiting thread will be woken up.

A thread-based example could be:

```c
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_CUSTOMERS 6
#define NUM_TABLES 3

sem_t tables;

void *customer(void *arg) {
    int id = *(int *)arg;
    printf("Customer %d is waiting for a table.\n", id);

    // Request one table.
    sem_wait(&tables);

    printf("Customer %d got a table.\n", id);
    // Simulate eating.
    sleep(2);
    printf("Customer %d left the table.\n", id);

    // Release the table.
    sem_post(&tables);

    return NULL;
}

int main(void) {
    pthread_t customers[NUM_CUSTOMERS];
    int ids[NUM_CUSTOMERS];

    // Initialize the semaphore.
    // The second argument is 0, which means this semaphore is shared between threads.
    // The initial value is NUM_TABLES.
    sem_init(&tables, 0, NUM_TABLES);

    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        ids[i] = i + 1;
        pthread_create(&customers[i], NULL, customer, &ids[i]);
    }

    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_join(customers[i], NULL);
    }

    sem_destroy(&tables);

    return 0;
}
```

In this example, at most `3` customer threads can use a table at the same time. Other customer threads are blocked until a table becomes available.

```c
sem_init(&tables, 0, NUM_TABLES);
```

The second argument is `0`, which means the semaphore is shared between **threads in the same process**.

So, for thread synchronization, the basic pattern is:

```c
sem_t S;
sem_init(&S, 0, initial_value);
sem_wait(&S);   // P(S): request resource
// Critical section or resource usage.
sem_post(&S);   // V(S): release resource
sem_destroy(&S);
```

#### A.2.4 Using POSIX Semaphores Between Processes

Using semaphores between processes is slightly more complicated. Unlike threads, processes do not normally share memory. If we create a normal global semaphore like this:

```c
sem_t tables;
```

then each process may have its own private copy of `tables`. Updating the semaphore in one process may not affect the semaphore in another process. Therefore, when we use semaphores between processes, we need a semaphore that can be shared across process boundaries. There are two common ways to do this:

1. Use a **named semaphore**
2. Put an unnamed semaphore inside **shared memory**

For now, we will focus on named semaphores because they are easier to understand.

A named semaphore is identified by a name, such as:

```c
"/wcdonalds_tables"
```

Different processes can open the same named semaphore by using the same name.

For example:

```c
sem_t *tables = sem_open("/wcdonalds_tables", O_CREAT, 0644, 3);
```

This creates or opens a semaphore named `"/wcdonalds_tables"` with an initial value of `3`.

Now multiple processes can use the same semaphore.

```c
sem_wait(tables);  // Request a table.

// Use the shared resource.

sem_post(tables);  // Release the table.
```

A process-based example could look like this:

```c
#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    sem_t *tables;

    // Create or open a named semaphore.
    // Initial value: 3 available tables.
    tables = sem_open("/wcdonalds_tables", O_CREAT, 0644, 3);

    if (tables == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Child process.
        printf("Child process is waiting for a table.\n");

        sem_wait(tables);

        printf("Child process got a table.\n");
        sleep(2);
        printf("Child process left the table.\n");

        sem_post(tables);

        sem_close(tables);
        return 0;
    } else {
        // Parent process.
        printf("Parent process is waiting for a table.\n");

        sem_wait(tables);

        printf("Parent process got a table.\n");
        sleep(2);
        printf("Parent process left the table.\n");

        sem_post(tables);

        wait(NULL);

        sem_close(tables);

        // Remove the named semaphore after we are done using it.
        sem_unlink("/wcdonalds_tables");
    }

    return 0;
}
```

The key difference is that `tables` is not a normal local or global variable. It is a pointer returned by `sem_open()`:

```c
sem_t *tables = sem_open("/wcdonalds_tables", O_CREAT, 0644, 3);
```

Both the parent and child processes can operate on the same named semaphore. For process synchronization, the basic pattern is:

```c
sem_t *S;

S = sem_open("/some_name", O_CREAT, 0644, initial_value);

sem_wait(S);   // P(S): request resource

// Critical section or resource usage.

sem_post(S);   // V(S): release resource

sem_close(S);
sem_unlink("/some_name");
```

#### A.2.5 Thread Semaphores vs. Process Semaphores

The `P()` and `V()` operations are conceptually the same for both threads and processes. The difference is how the semaphore itself is shared.

For threads:

```c
sem_t S;
sem_init(&S, 0, initial_value);
```

The semaphore can be a normal variable because threads share the same address space.

For processes:

```c
sem_t *S;
S = sem_open("/some_name", O_CREAT, 0644, initial_value);
```

The semaphore must be shared between processes. A named semaphore is one way to do this.

In both cases, the purpose of the semaphore is the same:

```c
sem_wait(S);  // Wait until the resource is available.
sem_post(S);  // Release the resource.
```

The semaphore protects access to a limited resource and prevents too many threads or processes from using that resource at the same time.

#### A.2.6 Important Notes

A semaphore does not protect only code. It usually represents access to a limited resource.

For example, the resource could be:

- tables in a restaurant
- printers in a computer lab
- empty slots in a buffer
- available food items in WcDonald's
- access permission to a critical section

Also, the meaning of `sem_wait()` and `sem_post()` depends on the resource.

For reusable resources, such as tables, the same thread or process usually calls both:

```c
sem_wait(&tables);

// Use the table.

sem_post(&tables);
```

For produced resources, such as fries, one thread or process may produce the resource, while another thread or process consumes it.

For example:

```c
// Kitchen thread:
sem_post(&fries);   // One order of fries is ready.

// Customer thread:
sem_wait(&fries);   // Take one available order of fries.
```

So semaphores can be used for both mutual exclusion and resource counting.

#### A.2.7 Binary Semaphore Is a Mutex

A semaphore whose value is only used as `0` or `1` is called a **binary semaphore**.

A binary semaphore can be used like a mutex. When the value is `1`, the resource is available. When a thread calls `sem_wait()`, the value becomes `0`, meaning the resource is now locked. When the thread finishes using the resource, it calls `sem_post()`, and the value becomes `1` again. In other words, for this simple case, a binary semaphore is a mutex: it allows only one thread or process to enter the critical section at a time. However, in real POSIX thread programs, `pthread_mutex_t` is often preferred when we only need a lock. A semaphore is more general because it can also be used for signaling and for counting multiple available resources.

---

### A.3 Exercise: Producer-Consumer

The producer reads queries from `stdin` and calls `enqueue()`. Each consumer blocks in `dequeue()` until the semaphore says work is available.

```text
mutex      protects the queue structure: head, tail, node links
semaphore  counts how many queue items are available
```

The uploaded scaffold already has a linked-list queue with `head` and `tail`, an `enqueue()` function, a `dequeue()` function, and 3 worker threads. The missing pieces are the mutex, the semaphore, shutdown handling, and cleanup. The scaffold also has one small bug: it uses `NUM_CONSUMERS` near the end, but the variable actually defined is `NUM_WORKERS`.

> [!IMPORTANT]
> Refer to [`producer-consumer/program.c`](./Codes/producer-consumer/program.c) for the solution and demo to this exercise.
>
> Put `program.c` and the provided `database` helper in the same directory. Then run:
>
> ```bash
> chmod +x database
> gcc -std=c11 -Wall -Wextra -pedantic -g program.c -o program -pthread
> ```

#### A.3.1 Running the Demo

To run the demo, run `./program` and then type some mock database queries:

```sql
SELECT * FROM users
INSERT INTO users VALUES (1)
UPDATE users SET status=1
```

Press `Ctrl+D` to close stdin. That tells the producer there are no more queries.

Or run a quick piped demo:

```bash
printf 'SELECT * FROM users\nINSERT INTO users VALUES (1)\nUPDATE users SET status=1\n' | ./program
```

#### A.3.2 Explanation for Notes

The queue is shared by all threads:

```c
static Node *head = NULL;
static Node *tail = NULL;
```

The mutex protects the queue pointers:

```c
pthread_mutex_lock(&queue_lock);
/* modify head and tail */
pthread_mutex_unlock(&queue_lock);
```

The semaphore counts available items:

```c
sem_t items_available;
```

It starts at `0` because the queue is initially empty:

```c
sem_init(&items_available, 0, 0);
```

When the producer enqueues a query, it posts the semaphore:

```c
sem_post(&items_available);
```

When a consumer wants work, it waits:

```c
sem_wait(&items_available);
```

So consumers do not busy-wait. They sleep until there is work.

#### A.3.3 Why Both Mutex and Semaphore Are Needed?

The semaphore answers:

```text
Is there at least one item available?
```

The mutex answers:

```text
Can I safely modify the queue pointers right now?
```

The semaphore alone does not protect `head` and `tail`. Two consumers could wake up and corrupt the linked list if they both remove nodes at the same time.

The mutex alone does not let consumers sleep until work exists. Without the semaphore, consumers would need to repeatedly check whether the queue is empty.

So the design is:

```text
semaphore -> availability of work
mutex     -> safe queue access
```

#### A.3.4 Shutdown Using Sentinel Values

When `stdin` closes, the producer needs to stop the consumers.

This solution enqueues `NULL` as a sentinel:

```c
enqueue(NULL);
```

Each consumer checks:

```c
if (query == NULL) {
    return NULL;
}
```

There are 3 workers, so we enqueue 3 sentinels:

```c
for (int i = 0; i < NUM_WORKERS; i++) {
    enqueue(NULL);
}
```

If we only enqueued one `NULL`, only one consumer would stop. The others could block forever in `sem_wait()`.

#### A.3.5 Common Mistakes

A common mistake is holding the mutex while running the database query. That would make only one consumer useful at a time.

Wrong idea:

```c
lock queue
dequeue query
run database
unlock queue
```

Correct idea:

```text
lock only while removing from the queue
unlock before running the slow database command
```

Another common mistake is posting the semaphore before the item is actually in the queue. The safe order is:

```text
lock queue
insert node
unlock queue
post semaphore
```

Also, the scaffold uses `NUM_CONSUMERS`, but the program defines `NUM_WORKERS`. Use `NUM_WORKERS`.

---

### A.4 Exercise: Dining Semaphores

This is another version of the dining philosophers problem. Previously, we avoided deadlock by using a locking hierarchy. We always lock lower-numbered chopstick first and then lock higher-numbered chopstick. This time, the main extra tool is a semaphore. The chopsticks are still mutexes, because each chopstick can only be held by one philosopher at a time. The semaphore limits how many philosophers are allowed to try eating at once.

For `THINKERS = 5`, we initialise the semaphore to `THINKERS / 2` which gives `2` so at most two philosophers can be in the "trying to eat" region at once.

The classic deadlock happens when every philosopher picks up one chopstick and waits for the next one:

```text
P0 holds chopstick 0 and waits for 1
P1 holds chopstick 1 and waits for 2
P2 holds chopstick 2 and waits for 3
P3 holds chopstick 3 and waits for 4
P4 holds chopstick 4 and waits for 0
```

Now everyone is waiting in a cycle. The semaphore prevents too many philosophers from attempting to eat at the same time. If only `N / 2` philosophers can enter the dining section, then the "everyone holds one chopstick" cycle cannot form. So we could say that the semaphore protects the act of dining that only `N / 2` philosophers may try to dine at once, and the chopstick mutexes still protect the individual chopsticks.

> [!IMPORTANT]
> Refer to [`phils.c`](./Codes/phils.c) for the solution to this exercise.
> You need to use the `-pthread` flag when compiling.
>
> The original scaffold uses `while (true)`, which is fine for an endless simulation. However, if the loop is infinite, `pthread_join()` never returns and the mutex destroy code is never reached.
>
> Just like the previous version, I also provide a `for` loop to see the result of fixed number of meals. Refer to line `29-32` in my codes, uncomment the `for` loop and comment the `while (True)` loop.

#### A.4.1 Step 1: Initialise the Semaphore

The scaffold has:

```c
static sem_t dining_counter;
```

So the correct `sem_init()` call for multi-threads is:

```c
// &dining_counter   address of the semaphore
// 0                 shared between threads in this process, not processes
// THINKERS / 2      initial number of permits
sem_init(&dining_counter, 0, THINKERS / 2);
```

For `THINKERS = 5`, this starts with 2 permits.

#### A.4.2 Step 2: Use the Semaphore in `dine()`

Before a philosopher tries to pick up chopsticks, it must enter through the semaphore:

```c
sem_wait(&dining_counter);
```

After it finishes eating and releases both chopsticks, it gives the permit back:

```c
sem_post(&dining_counter);
```

The basic flow is:

```text
wait for permission to dine
lock left chopstick
lock right chopstick
eat
unlock right chopstick
unlock left chopstick
release dining permit
```

#### A.4.3 Explanation for Notes

There are two different shared resources in this solution. The chopsticks are protected by mutexes:

```c
pthread_mutex_lock(&chopsticks[left]);
pthread_mutex_lock(&chopsticks[right]);
```

This prevents two philosophers from holding the same chopstick at the same time.

The act of dining is limited by the semaphore:

```c
sem_wait(&dining_counter);
```

This prevents too many philosophers from trying to acquire chopsticks at the same time.

After eating, the philosopher releases everything:

```c
pthread_mutex_unlock(&chopsticks[right]);
pthread_mutex_unlock(&chopsticks[left]);
sem_post(&dining_counter);
```

The order matters. The philosopher should release the chopsticks before allowing another philosopher into the dining section.

On the other hand, the scaffold uses `while (true)` so the philosopher threads never finish. That means `pthread_join()` will also never return, and the cleanup code at the end of `main()` will not normally run. That is fine for an endless simulation, but for testing it can be easier to replace the infinite loop with a fixed number of meals:

```c
for (int meal = 0; meal < 3; meal++) {
    ...
}
```

Then the program can finish and destroy the mutexes and semaphore cleanly.

---

### A.5 Exercise: The Tortoise and the Hare

The goal is to force the tortoise to finish first using a semaphore.

The shared finish function is:

```c
int reach_finish() {
    if (race_won == 0) {
        race_won = 1;
        return 1;
    }
    return 0;
}
```

So whoever calls `reach_finish()` first wins.

To make sure the tortoise always wins (poor hare 🐰😭):

```text
tortoise reaches finish
tortoise calls reach_finish()
tortoise posts the semaphore
hare waits on the semaphore before calling reach_finish()
```

The semaphore starts at `0`, meaning the hare is blocked until the tortoise releases it.

> [!IMPORTANT]
> Refer to [`fable.c`](./Codes/fable.c) for the solution to this exercise.
> You need to use the `-pthread` flag when compiling.

#### A.5.1 Step 1: Add a Semaphore

We need one semaphore which starts at `0`:

```c
sem_t tortoise_done;

// &tortoise_done   address of the semaphore
// 0                shared between threads in this process
// 0                initial value: hare must wait
sem_init(&tortoise_done, 0, 0);
```

#### A.5.2 Step 2: Tortoise Posts When Finished

The tortoise moves until it reaches `DISTANCE`. Then it calls:

```c
reach_finish()
```

After that, it wakes the hare:

```c
sem_post(&tortoise_done);
```

The order matters. The tortoise should post only after it has claimed the finish line.

#### A.5.3 Step 3: Hare Waits before Finishing

The hare can run quickly, but before calling `reach_finish()`, it must wait:

```c
sem_wait(&tortoise_done);
```

So even if the hare reaches the end first in terms of progress, it cannot claim victory until the tortoise has already won.

---

### A.6 Condition Variables

A condition variable lets a thread sleep until some shared condition becomes true.

Without a condition variable, a thread might poll:

```c
while (!ready) {
    sleep(1);
}
```

This works poorly because:
1. it wastes time checking repeatedly
2. it may notice the change late
3. it uses fixed sleep intervals instead of waking exactly when needed

A condition variable gives a cleaner pattern:
1. waiter sleeps while condition is false
2. signaller changes shared state
3. signaller wakes waiter
4. waiter continues when condition is true

The important point is that a condition variable is not the condition itself. The condition is some shared state, such as:

```c
int ready = 0;
int queue_size = 0;
int work_available = 0;
```

The condition variable is just the sleeping/waking mechanism.

> [!IMPORTANT]
> A condition variable is always used with a mutex. The mutex protects the shared state being checked.

For example:

```c
pthread_mutex_t lock;
pthread_cond_t cv;

int ready = 0;
```

The shared condition is:

```c
ready == 1
```

The mutex protects `ready`.

This matters because checking the condition and going to sleep must be done safely. Otherwise, a lost wake-up can happen:

```text
Thread A checks ready and sees ready == 0
Thread B sets ready = 1 and signals
Thread A goes to sleep
Thread A sleeps forever because the signal already happened
```

Condition variables do not remember old signals.

So the correct waiting operation must be atomic: "release mutex and sleep". That is exactly what `pthread_cond_wait()` does.

> [!IMPORTANT]
> Refer to [`condition_variables.c`](./Codes/condition_variables.c) for demonstration.
> You need to use the `-pthread` flag when compiling.

#### A.6.1 POSIX Condition Variable Functions

The POSIX type of a condition variable is `pthread_cond_t` and it could be initialized by:

```c
// 1. Statically
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

// 2. Dynamically
pthread_cond_t cv;
pthread_cond_init(&cv, NULL);
```

Last when finished call:

```c
pthread_cond_destroy(&cv);
```

In running, the main wait function is:

```c
pthread_cond_wait(&cv, &lock);
```

This must be called while holding the mutex. It does three things:
1. releases the mutex
2. puts the thread to sleep on the condition variable
3. when woken, reacquires the mutex before returning

There is also a timed version:

```c
pthread_cond_timedwait(&cv, &lock, &deadline);
```

This waits until either the condition variable is signalled or the timeout expires.

To wake waiters:

```c
pthread_cond_signal(&cv);       // wakes one waiting thread.
pthread_cond_broadcast(&cv);    // wakes all waiting threads.
```

Use `pthread_cond_signal()` when one waiter should wake.

Example:

```text
producer adds one item to a queue
one consumer can take that item
```

Use `pthread_cond_broadcast()` when many waiters might now be able to continue.

Example:

```text
shutdown flag becomes true
all worker threads should wake and exit
or a state change may satisfy several different waiting threads
```

Broadcast is more expensive because all waiting threads wake up, and many may immediately go back to sleep after re-checking the condition.

That is another reason the `while` loop matters.

#### A.6.2 The Standard Waiting Pattern

The waiting thread should use a `while` loop:

```c
pthread_mutex_lock(&lock);

while (!condition_is_true) {
    pthread_cond_wait(&cv, &lock);
}

/* condition is true, and this thread holds the mutex */

pthread_mutex_unlock(&lock);
```

Use `while`, not `if`.

This is important because a thread can wake up and still not be able to continue.

That can happen because:

```text
another thread consumed the resource first
the signal was meant for a different waiter
the thread woke spuriously
the condition changed again before this thread reacquired the mutex
```

So this is unsafe:

```c
pthread_mutex_lock(&lock);

if (!condition_is_true) {
    pthread_cond_wait(&cv, &lock);
}

/* condition may not actually be true here */

pthread_mutex_unlock(&lock);
```

> [!NOTE]
> The safe rule is always wait in a while loop that re-checks the condition.

#### A.6.3 The Standard Signalling Pattern

The signalling thread should change the shared state while holding the mutex:

```c
pthread_mutex_lock(&lock);

/* change shared state */
condition_is_true = 1;

pthread_cond_signal(&cv);

pthread_mutex_unlock(&lock);
```

The important order is:

```text
lock
change shared state
signal
unlock
```

The signal does not unlock the mutex. If a waiting thread wakes up, it still cannot return from `pthread_cond_wait()` until it has reacquired the mutex.

So this can happen:

```text
signaller sets ready = 1
signaller signals cv
waiter wakes up
waiter tries to reacquire lock
signaller unlocks
waiter reacquires lock and continues
```

#### A.6.4 Difference between Waiting on a Mutex and Waiting on a Condition Variable

Waiting on a mutex means "I want exclusive access to this protected data." If another thread holds the mutex, this thread waits until the mutex becomes available. But once the mutex is available, the thread continues. A mutex does not know whether some higher-level condition is true.

Waiting on a condition variable means "I want to sleep until some shared state changes in a useful way." For example:

```c
while (queue_empty) {
    pthread_cond_wait(&cv, &lock);
}
```

This means:

```text
I do not just need the lock.
I need the queue to become non-empty.
```

So the distinction is:

```text
mutex wait       -> wait until I can enter the critical section
condition wait   -> wait until the program state I need becomes true
```

A mutex protects shared state. A condition variable waits for shared state to change.

#### A.6.5 Producer-Consumer Intuition

Condition variables are often used for producer-consumer queues.

The shared condition might be `queue_size > 0` and a consumer should **not** poll:

```c
while (queue_size == 0) {
    sleep(1);
}
```

Instead, it waits:

```c
pthread_mutex_lock(&lock);

while (queue_size == 0) {
    pthread_cond_wait(&not_empty, &lock);
}

/* remove item from queue */

pthread_mutex_unlock(&lock);
```

The producer adds an item and signals:

```c
pthread_mutex_lock(&lock);

/* add item to queue */
queue_size++;

pthread_cond_signal(&not_empty);

pthread_mutex_unlock(&lock);
```

This means consumers sleep while there is no work and wake when the producer adds work.
