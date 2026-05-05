## COMP2017 2026 S1 Week 10 Tutorial A

<table><tbody>
  <tr><td><b>Tutor</b></td><td>Hao Ren</td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au">hao.ren@sydney.edu.au</a></td></tr>
</tbody></table>

- [COMP2017 2026 S1 Week 10 Tutorial A](#comp2017-2026-s1-week-10-tutorial-a)
  - [A.1 Asynchronous I/O: Interrupt-Based vs `epoll`-Based](#a1-asynchronous-io-interrupt-based-vs-epoll-based)
    - [A.1.1 Interrupt-Based Asynchronous I/O](#a11-interrupt-based-asynchronous-io)
      - [Pros and Cons](#pros-and-cons)
    - [A.1.2 `epoll`-Based Asynchronous I/O](#a12-epoll-based-asynchronous-io)
      - [Readiness, Not Automatic Reading](#readiness-not-automatic-reading)
      - [Non-Blocking Mode](#non-blocking-mode)
    - [A.1.3 Gentle Corrections And Important Distinctions](#a13-gentle-corrections-and-important-distinctions)
      - [Hardware Interrupts Do Not Directly Run User Code](#hardware-interrupts-do-not-directly-run-user-code)
      - [`epoll` Does Not Poll in a Wasteful Loop](#epoll-does-not-poll-in-a-wasteful-loop)
      - [The "Slight Latency" Point](#the-slight-latency-point)
      - [Regular Files Are Different](#regular-files-are-different)
  - [A.2 `select()`, `poll()` and `epoll()`](#a2-select-poll-and-epoll)
    - [A.2.1 Readiness-Based I/O](#a21-readiness-based-io)
    - [A.2.2 `select()`](#a22-select)
      - [Limitations of `select()`](#limitations-of-select)
    - [A.2.3 `poll()`](#a23-poll)
      - [Limitation of `poll()`](#limitation-of-poll)
    - [A.2.4 `epoll()`](#a24-epoll)
      - [Step 1: Create an `epoll` Instance](#step-1-create-an-epoll-instance)
      - [Step 2: Register a File Descriptor](#step-2-register-a-file-descriptor)
      - [Step 3: Wait for Events](#step-3-wait-for-events)
      - [Why `epoll()` Scales Better](#why-epoll-scales-better)
      - [Level-Triggered and Edge-Triggered `epoll`](#level-triggered-and-edge-triggered-epoll)
    - [A.2.5 `select()` vs `poll()` vs `epoll()`](#a25-select-vs-poll-vs-epoll)
      - [`select()`](#select)
      - [`poll()`](#poll)
      - [`epoll()`](#epoll)
    - [A.2.6 Non-Blocking File Descriptors](#a26-non-blocking-file-descriptors)
  - [A.3 `socketpair()`](#a3-socketpair)
  - [A.4 Exercise: eToll](#a4-exercise-etoll)
    - [A.3.1 Should `read()` Be Blocking or Non-Blocking?](#a31-should-read-be-blocking-or-non-blocking)
    - [A.3.2 Short Summary](#a32-short-summary)
  - [A.4 Shared Memory](#a4-shared-memory)
    - [A.4.1 Threads vs Processes](#a41-threads-vs-processes)
    - [A.4.2 `mmap()`](#a42-mmap)
    - [A.4.3 `MAP_SHARED` vs `MAP_PRIVATE`](#a43-map_shared-vs-map_private)
    - [A.4.4 Shared Memory between Related Processes](#a44-shared-memory-between-related-processes)
    - [A.4.5 Shared Memory between Independent Processes](#a45-shared-memory-between-independent-processes)
      - [Example: Writer and Reader Programs](#example-writer-and-reader-programs)
    - [A.4.6 Sharing Data Does Not Synchronize Access](#a46-sharing-data-does-not-synchronize-access)
  - [A.5 Exercise: Word Count II](#a5-exercise-word-count-ii)
    - [A.5.1 Step 1: Open and Map the File](#a51-step-1-open-and-map-the-file)
    - [A.5.2 Step 2: Search Case-Insensitively](#a52-step-2-search-case-insensitively)
    - [A.5.3 Step 3: Print Matching Lines and Count Occurrences](#a53-step-3-print-matching-lines-and-count-occurrences)
    - [A.5.4 Notes](#a54-notes)
  - [A.6 Semaphore](#a6-semaphore)
    - [A.6.1 Basic POSIX Semaphore Functions](#a61-basic-posix-semaphore-functions)
    - [A.6.2 Simple Writer Idea](#a62-simple-writer-idea)
    - [A.6.3 Simple Reader Idea](#a63-simple-reader-idea)
    - [A.6.4 Important Idea](#a64-important-idea)
    - [A.6.5 Binary Semaphore](#a65-binary-semaphore)
    - [A.6.6 Counting Semaphore](#a66-counting-semaphore)
    - [A.6.7 Cleanup](#a67-cleanup)
  - [A.7 Advanced Exercise: Real-time Stock Ticker](#a7-advanced-exercise-real-time-stock-ticker)
    - [A.7.1 Codes and Usages](#a71-codes-and-usages)
    - [A.7.2 How Does the Subscriber Know There Is an Update?](#a72-how-does-the-subscriber-know-there-is-an-update)
    - [A.7.3 Should We Use Signals?](#a73-should-we-use-signals)
    - [A.7.4 Design Choice](#a74-design-choice)
    - [A.7.5 Notes](#a75-notes)
    - [A.7.6 What If We Used Signals Instead?](#a76-what-if-we-used-signals-instead)
    - [A.7.7 Common Mistakes](#a77-common-mistakes)
  - [A.7 Exercise: `poll` / `epoll` Monitoring Children](#a7-exercise-poll--epoll-monitoring-children)
    - [A.7.1 `poll` Version](#a71-poll-version)
    - [A.7.2 `epoll` Version](#a72-epoll-version)
    - [A.7.3 `pause` Note](#a73-pause-note)
    - [A.7.4  Short Summary](#a74--short-summary)

---

### A.1 Asynchronous I/O: Interrupt-Based vs `epoll`-Based

Asynchronous I/O is about avoiding this pattern:

```c
read(fd, buffer, size);   // block here until data arrives
```

Instead of waiting on one operation and doing nothing, the program arranges to be notified when I/O is ready.

There are two common styles:
1. **interrupt/signal/callback style**: something interrupts normal control flow when data arrives
2. **`epoll` style**: the program waits in one event loop for many file descriptors

The main difference is where the control flow lives.

- With interrupt-style I/O, the program is interrupted and a handler runs.
- With `epoll`, the program stays in control and asks the kernel, "which file descriptors are ready now?"

#### A.1.1 Interrupt-Based Asynchronous I/O

In a true hardware sense, devices interrupt the CPU when something happens. However, user programs do not normally run hardware interrupt handlers directly. The kernel handles the hardware interrupt first.

At the user-program level, "interrupt-based asynchronous I/O" usually means something like:

```text
a signal handler
a callback
a notification routine
```

The idea is:

```text
data arrives
kernel/device notices
program is notified
handler or callback runs
```

For example, a signal-based design might use a signal such as `SIGIO` to notify the program that I/O is possible.

This style feels like "tell me immediately when something happens." The program does not keep checking the file descriptor manually. Instead, some event interrupts normal execution. This can be useful for quick notifications, but it makes the program harder to reason about because the handler can run at awkward times. For example, the signal might arrive while the program is halfway through updating a data structure.

This is a simplified signal-style pattern:

```c
static volatile sig_atomic_t got_io = 0;

static void handler(int sig) {
    (void)sig;
    got_io = 1;
}

int main(void) {
    /*
     * Install signal handler here.
     * Configure fd to generate signal notifications.
     */

    while (1) {
        if (got_io) {
            got_io = 0;

            /*
             * Do the actual read in normal program code,
             * not inside the signal handler.
             */
        }

        /*
         * Do other work.
         */
    }
}
```

The important idea is that the handler should stay small.

A signal handler should usually not do complex work such as:

```c
printf("data arrived\n");
malloc(100);
read_large_message();
update_complex_data_structure();
```

A safer handler often just sets a flag or writes one byte to a pipe.

> [!NOTE]
>
> A `while (1)` loop does **not automatically mean polling**. Many programs use an infinite loop as their main event loop. The difference is what the loop does:
>
> ```c
> while (1) {
>     if (got_io) {
>         got_io = 0;
>         read(fd, buf, size);
>     }
>
>     do_other_work_or_sleep();
> }
> ```
>
> With interrupt-style notification, the program does **not** keep asking the device:
>
> ```text
> Is data ready? Is data ready? Is data ready?
> ```
>
> Instead, the OS sends a signal when I/O is ready, and the signal handler sets a flag:
>
> ```c
> got_io = 1;
> ```
>
> Then the main loop notices the flag and handles the I/O.
>
> Important point:
>
> ```text
> Good: the loop does useful work or sleeps.
> Bad:  the loop only checks got_io repeatedly.
> ```
>
> So interrupt-style notification avoids polling the device, but you must still avoid a tight busy loop that wastes CPU.

##### Pros and Cons

Interrupt-style notification can respond quickly because the program is notified when the event happens.

It can also avoid wasting CPU time because the program does not need to repeatedly poll:

```text
is data ready?
is data ready?
is data ready?
```

Instead, it can wait for real notifications.

The main downside is complexity.

Handlers can run asynchronously, meaning they may interrupt the program almost anywhere. This creates problems such as:

```text
reentrancy bugs
shared-state races
unsafe function calls inside handlers
hard-to-reproduce timing bugs
```

For many file descriptors, this style can also become awkward. If I have hundreds or thousands of sockets, I usually do not want a separate signal-style path for each one.

#### A.1.2 `epoll`-Based Asynchronous I/O

`epoll` is a Linux mechanism for waiting on many file descriptors efficiently. The usual pattern is:

```text
create an epoll instance
register file descriptors with it
call epoll_wait()
handle the file descriptors that became ready
repeat
```

The key function is:

```c
epoll_wait(epfd, events, max_events, timeout);
```

This blocks until at least one registered file descriptor is ready, or until the timeout expires. When it returns, it gives the program a list of ready file descriptors. This style feels like "I have many possible I/O sources. Wake me when at least one is ready."

Instead of installing many separate handlers, the program has one main event loop. The event loop is usually shaped like this:

```c
while (1) {
    int n = epoll_wait(epfd, events, MAX_EVENTS, -1);

    for (int i = 0; i < n; i++) {
        int fd = events[i].data.fd;

        if (events[i].events & EPOLLIN) {
            /*
             * fd is ready to read.
             */
        }

        if (events[i].events & EPOLLOUT) {
            /*
             * fd is ready to write.
             */
        }
    }
}
```

This is easier to structure because all event handling happens in one place.

##### Readiness, Not Automatic Reading

A subtle but important point:

```text
epoll tells me an fd is ready.
epoll does not read the data for me.
```

After `epoll_wait()` returns, the program still needs to call `read(fd, buffer, size)` or `write(fd, buffer, size)` so `epoll` is readiness-based I/O, not "the kernel has already copied all the data into my program."

##### Non-Blocking Mode

In an event loop, file descriptors are usually set to non-blocking mode.

The reason is simple: even if `epoll` says a file descriptor is ready, a later operation might still block in some cases, especially with multiple readers, partial reads, edge-triggered mode, or changing state.

With non-blocking I/O, if there is currently no more data, `read()` returns `-1` with `errno == EAGAIN` or `EWOULDBLOCK`.

The common pattern is:

```c
while (1) {
    ssize_t n = read(fd, buf, sizeof buf);

    if (n > 0) {
        /*
         * Process bytes.
         */
    } else if (n == 0) {
        /*
         * EOF: peer closed connection.
         */
        close(fd);
        break;
    } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
        /*
         * No more data available right now.
         */
        break;
    } else {
        /*
         * Real error.
         */
        perror("read");
        close(fd);
        break;
    }
}
```

This is especially important for edge-triggered `epoll`.

#### A.1.3 Gentle Corrections And Important Distinctions

##### Hardware Interrupts Do Not Directly Run User Code

It is slightly simplified to say that the hardware "interrupts your program" and your handler runs immediately.

More precisely:

```text
hardware interrupt happens
kernel handles it
kernel may later notify the user process
user process handles that notification
```

A user-level signal handler is not the same thing as a hardware interrupt handler.

##### `epoll` Does Not Poll in a Wasteful Loop

The name sounds like polling, but `epoll_wait()` is not the same as repeatedly checking every file descriptor manually.

This inefficient style would be:

```text
check fd 1
check fd 2
check fd 3
check fd 4
repeat forever
```

With `epoll`, the kernel tracks readiness and wakes the program when registered descriptors are ready. So `epoll` is usually efficient for many descriptors.

##### The "Slight Latency" Point

It is true that the program only handles events when control reaches the event loop. But while the program is blocked in `epoll_wait()`, the kernel can wake it as soon as an event is ready. So the latency is usually not because `epoll_wait()` is slow. It usually comes from:

```text
the event loop being busy with other work
the program not returning to epoll_wait quickly
scheduler delays
large batches of events
```

In practice, `epoll` is commonly used for high-performance network servers.

##### Regular Files Are Different

`epoll` is mainly useful for file descriptors where "readiness" changes in a meaningful way, such as:

```text
sockets
pipes
terminals
eventfd
timerfd
signalfd
```

Regular disk files usually behave differently because they are often considered always ready for reading or writing. So `epoll` is usually not the main tool for asynchronous disk-file I/O.

---

### A.2 `select()`, `poll()` and `epoll()`

`select()`, `poll()`, and `epoll()` are used when a program needs to wait for I/O on multiple file descriptors.

Instead of doing this:

```c
read(fd1, buf, size);   // might block forever
read(fd2, buf, size);   // may never get reached
read(fd3, buf, size);
```

we ask the kernel: "tell me which file descriptors are ready for I/O." Then the program only calls `read()` or `write()` on file descriptors that are ready. This is important for servers and event-driven programs because one process can manage many sockets, pipes, or terminals without creating one process or thread per connection.

#### A.2.1 Readiness-Based I/O

These APIs are readiness-based. That means they do not usually perform the I/O operation for us. They only tell us that an operation should be possible without blocking.

For example, `fd` is ready for reading usually means a `read()` call should be able to make progress. Then the program still needs to call:

```c
read(fd, buf, sizeof buf);
```

Similarly, `fd` is ready for writing usually means a `write()` call should be able to write at least some data.

In short, we could say `select()/poll()/epoll()` report readiness and `read/write` perform the actual I/O.

#### A.2.2 `select()`


`select()` is one of the **older ways** to wait on multiple file descriptors. The program creates sets of file descriptors:

```text
read set         fds we want to read from
write set        fds we want to write to
exception set    fds with exceptional conditions
```

Then it calls:

```c
int n = select(max_fd + 1, &read_fds, &write_fds, &err_fds, &timeout);
```

The first argument `max_fd + 1` is important which tells `select()` how many file descriptor numbers to check. For example, if the largest fd is `7`, then the first argument should be `8`.

This is a simple `select()` shape:

```c
fd_set read_fds;

FD_ZERO(&read_fds);
FD_SET(fd1, &read_fds);
FD_SET(fd2, &read_fds);

int max_fd = fd1 > fd2 ? fd1 : fd2;

int nready = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

if (nready == -1) {
    perror("select");
    return 1;
}

if (FD_ISSET(fd1, &read_fds)) {
    /* fd1 is ready to read */
}

if (FD_ISSET(fd2, &read_fds)) {
    /* fd2 is ready to read */
}
```

The basic pattern is:

1. clear `fd_set`
2. add `fds`
3. call `select`
4. check which `fds` are still set

> [!NOTE]
> `select()` modifies the sets. After `select()` returns, the sets no longer contain the original full list. They contain only the file descriptors that are ready.
>
> So in a loop, this is **wrong**:
>
> ```c
> fd_set read_fds;
>
> FD_ZERO(&read_fds);
> FD_SET(fd, &read_fds);
>
> while (1) {
>     select(fd + 1, &read_fds, NULL, NULL, NULL);
>
>     if (FD_ISSET(fd, &read_fds)) {
>         read(fd, buf, sizeof buf);
>     }
> }
> ```

The set may be changed after the first call.

The **safer** pattern is to keep a master set and copy it:

```c
fd_set master;
FD_ZERO(&master);
FD_SET(fd, &master);

while (1) {
    fd_set read_fds = master;

    int nready = select(fd + 1, &read_fds, NULL, NULL, NULL);

    if (nready == -1) {
        perror("select");
        break;
    }

    if (FD_ISSET(fd, &read_fds)) {
        read(fd, buf, sizeof buf);
    }
}
```

##### Limitations of `select()`

The main limitation is scalability. `select()` works with bitsets. On each call, the kernel checks file descriptor numbers up to `max_fd`. The program usually also loops through possible fd numbers to see which ones are ready. So if I am watching many file descriptors, this becomes expensive.

Another limitation is `FD_SETSIZE`. On many systems, `fd_set` can only represent file descriptors below a fixed compile-time limit, commonly around `1024`. If a file descriptor number is too large, it cannot safely be used with `FD_SET()`.

So `select()` is simple and portable, but not ideal for very large numbers of connections.

#### A.2.3 `poll()`

`poll()` is a **newer POSIX alternative** to `select()`. Instead of using bitsets, it uses an *array* of `struct pollfd`. Each entry says:

```text
which fd to watch
which events we care about
which events actually happened
```

The structure looks like this:

```c
struct pollfd {
    int   fd;         // the file descriptor
    short events;     // what we want to watch for
    short revents;    // what actually happened
};
```

A simple `poll()` shape looks like:

```c
#include <poll.h>

struct pollfd fds[2];

fds[0].fd = fd1;
fds[0].events = POLLIN;

fds[1].fd = fd2;
fds[1].events = POLLIN;

int nready = poll(fds, 2, -1);

if (nready == -1) {
    perror("poll");
    return 1;
}

if (fds[0].revents & POLLIN) {
    /* fd1 is ready to read */
}

if (fds[1].revents & POLLIN) {
    /* fd2 is ready to read */
}
```

The timeout argument works like this:

```text
-1    wait forever
0     return immediately
>0    wait up to that many milliseconds
```

By comparasion, `poll()` does not use `FD_SETSIZE`, so it does not have the same fixed bitset limit as `select()`. It also avoids the `max_fd + 1` issue. We pass:

```c
poll(fds, nfds, timeout);
```

where `nfds` is the number of entries in the array. That is often easier to reason about.

##### Limitation of `poll()`

`poll()` still scans the array. If we pass an array of 10,000 file descriptors, the kernel still has to inspect that array to find which descriptors are ready.

So `poll()` avoids some `select()` problems, but it is still generally `O(number of watched descriptors)`. This makes it better than `select()` in some ways, but not as scalable as `epoll()` for very large server workloads.

#### A.2.4 `epoll()`

`epoll()` is a *Linux-specific API* for monitoring many file descriptors efficiently.

> [!NOTE]
> `epoll()` is the **Linux-specific* API. For BSD / Mac OS you might need to refer to `kqueue()`.

Instead of passing the full set of file descriptors every time, we create an `epoll` instance and register file descriptors with it once. The basic pattern is:

```text
create epoll instance
add file descriptors to it
wait for ready events
handle ready file descriptors
repeat
```

##### Step 1: Create an `epoll` Instance

```c
int epfd = epoll_create1(0);

if (epfd == -1) {
    perror("epoll_create1");
    return 1;
}
```

The `epfd` is itself a file descriptor. It represents the kernel-side epoll object.

##### Step 2: Register a File Descriptor

```c
struct epoll_event ev;

ev.events = EPOLLIN;
ev.data.fd = sock;

if (epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev) == -1) {
    perror("epoll_ctl");
    return 1;
}
```

This means:

```text
watch sock for readability
when it becomes ready, return it through epoll_wait()
```

The common operations are:

```text
EPOLL_CTL_ADD   add a new fd
EPOLL_CTL_MOD   change what events we care about
EPOLL_CTL_DEL   remove an fd
```

##### Step 3: Wait for Events

```c
#define MAX_EVENTS 64

struct epoll_event events[MAX_EVENTS];

int nready = epoll_wait(epfd, events, MAX_EVENTS, -1);

if (nready == -1) {
    perror("epoll_wait");
    return 1;
}

for (int i = 0; i < nready; i++) {
    int fd = events[i].data.fd;

    if (events[i].events & EPOLLIN) {
        /* fd is ready to read */
    }
}
```

The key difference from `select()` and `poll()` is that `epoll_wait()` gives us an array of ready events. We do not need to scan every possible file descriptor ourselves.

##### Why `epoll()` Scales Better

With `select()` and `poll()`, the program repeatedly passes a full set or array of file descriptors to the kernel. However, with `epoll()`, the interest list is stored inside the kernel-side epoll object.

So the program can say:

```text
Here are the fds I care about.
Now wake me when some of them become ready.
```

This is better for large numbers of mostly idle connections, which is common in servers.

*A small correction to the common wording: `epoll()` is often described as `O(1)` per event, but the real performance depends on details such as the number of active events, kernel work, and how the program handles them. The important practical idea is that `epoll()` scales with ready activity much better than repeatedly scanning every watched fd.*

##### Level-Triggered and Edge-Triggered `epoll`

**Level-triggered mode** is the default.

It means keep reporting the `fd` as *ready* while the condition remains `true`. For example, if a socket has unread data, `epoll_wait()` may keep reporting it until the program reads the data. This is easier to use. Conceptually, "tell me whenever this fd is currently ready."

While **edge-triggered mode** is enabled with `EPOLLET`.

It means report when the readiness state *changes*. For example, the kernel may notify us when new data arrives, but it may not keep reminding us if we only read part of the data. Conceptually, "tell me when this fd changes from not ready to ready."

With edge-triggered `epoll`, the usual rule is:

```text
use non-blocking fds
read until EAGAIN or EWOULDBLOCK
```

Example shape:

```c
while (1) {
    ssize_t n = read(fd, buf, sizeof buf);

    if (n > 0) {
        /* process data */
    } else if (n == 0) {
        /* EOF */
        close(fd);
        break;
    } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
        /* no more data for now */
        break;
    } else {
        perror("read");
        close(fd);
        break;
    }
}
```

If we do not drain the `fd`, data can remain unread without another notification.

#### A.2.5 `select()` vs `poll()` vs `epoll()`

##### `select()`

`select()` is portable and good for small examples. It uses `fd_set` bitmasks:

```c
FD_SET(fd, &read_fds);
select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
```

But it has two big limitations:

```text
FD_SETSIZE limit
linear scanning up to max_fd
```

It also modifies the sets, so they usually need to be rebuilt or copied before each call.

##### `poll()`

`poll()` is also portable on POSIX systems and is usually cleaner than `select()`. It uses an array:

```c
struct pollfd fds[];
poll(fds, nfds, timeout);
```

It avoids `FD_SETSIZE` and does not require `max_fd + 1`. But it still scans the array, so it is not ideal for huge numbers of file descriptors.

##### `epoll()`

`epoll()` is Linux-specific and designed for large-scale event loops. It stores the watched file descriptors in a kernel object:

```c
epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
epoll_wait(epfd, events, MAX_EVENTS, timeout);
```

It is usually the best choice for Linux servers handling many connections. But it requires more setup than `select()` or `poll()`, and edge-triggered mode requires careful non-blocking I/O.

#### A.2.6 Non-Blocking File Descriptors

For event-driven programs, especially with `epoll`, file descriptors are usually set to non-blocking mode.

The reason is that one blocking `read()` or `write()` can stop the entire event loop.

For example, this is dangerous in an event loop:

```c
read(fd, buf, sizeof buf);
```

If it blocks, no other connection can be handled.

With non-blocking mode, `read()` can say:

```text
there is no more data right now
```

by returning `-1` with:

```c
errno == EAGAIN || errno == EWOULDBLOCK
```

Then the event loop can move on.

---

### A.3 `socketpair()`

> [!IMPORTANT]
> Refer to [`socketpair.c`](./Codes/socketpair.c) for demostrations.

`socketpair()` is used to create **two connected file descriptors** that can talk to each other.

We have already learned about `pipe()` last week:

```c
int fd[2];
pipe(fd);
```

A pipe is usually **one-way**:

```text
fd[1] ---> fd[0]
write      read
```

So one side writes, the other side reads.

`socketpair()` is similar, but more powerful. It creates two connected sockets, and **both sides can read and write**.

```text
sv[0] <----> sv[1]
read/write   read/write
```

So it is **two-way communication** in one call.

```c
#include <sys/socket.h>

int socketpair(int domain, int type, int protocol, int sv[2]);
```

Common usage:

```c
int sv[2];

socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
```

Meaning:

```text
AF_UNIX       communication on the same machine
SOCK_STREAM   reliable byte stream, like a pipe
0             default protocol
sv[0], sv[1]  the two connected endpoints
```

After this:

```c
write(sv[0], "hello", 5);
read(sv[1], buf, size);
```

and also:

```c
write(sv[1], "reply", 5);
read(sv[0], buf, size);
```

Both directions work.

After `fork()`, both the parent and child inherit both descriptors:

```text
Parent has sv[0] and sv[1]
Child has sv[0] and sv[1]
```

Usually we want:

```text
Parent uses sv[0]
Child uses sv[1]
```

So the parent closes `sv[1]`, and the child closes `sv[0]`.

This avoids confusion and helps EOF/close behavior work correctly.

`socketpair()` does not make I/O asynchronous.

This can still block:

```c
read(sv[0], buf, size);
```

If no data has been written by the other side, the program may wait. So `socketpair()` gives you a two-way connection, but normal `read()` and `write()` rules still apply.

---

### A.4 Exercise: eToll

The toll booth has several lanes. Each lane is represented by a file descriptor. Cars arrive on different lanes at different times and send payment messages. The server should:

```text
wait for activity on all lanes
process whichever lane has data
avoid busy-waiting
continue until all lanes are done
print process complete
```

The main tool is `epoll_wait()` which lets the parent sleep until at least one lane has data.

> [!IMPORTANT]
> Refer to [`etoll.c`](./Codes/etoll.c) for the solution to this exercise.

To keep the solution short, our solution uses `socketpair()` instead of named FIFOs.

Each lane gets one socket pair:
- parent reads from one end
- child writes to the other end

The parent registers all read ends with `epoll`.

#### A.3.1 Should `read()` Be Blocking or Non-Blocking?

Use **non-blocking reads**.

`epoll_wait()` tells us that a file descriptor is ready right now. But after reading the available data, there may be no more data left on that lane. If the lane is blocking and we try to keep reading, the server may get stuck waiting on one lane:

```text
lane 0 has one message
server reads it
server tries to read again
lane 0 has no more data yet
server blocks
lanes 1, 2, 3 cannot be handled
```

With non-blocking mode, the final read returns:

```text
-1 with errno == EAGAIN
```

That means:

```text
no more data available right now
go back to epoll_wait()
```

So the event loop stays responsive.

For level-triggered `epoll`, non-blocking mode is not always strictly required if we only do one careful read per event. But for a real event loop, non-blocking mode is the safer and more standard design.

#### A.3.2 Short Summary

The eToll server uses one file descriptor per lane and registers all lanes with `epoll`.

`epoll_wait()` lets the parent sleep until at least one lane has a payment message. The parent then reads from the ready lane, logs the payment, and returns to the event loop.

The lane file descriptors should be non-blocking. This prevents one empty or slow lane from blocking the entire toll booth and stopping other lanes from being processed.

---

### A.4 Shared Memory

Normally, different processes have separate memory images. That means if one process changes a normal variable, another process cannot see that change. Shared memory is a way to deliberately make part of memory visible to more than one process.

The key idea is that different processes map the same underlying memory object. Each process may see the shared region at a different virtual address, but the data behind that mapping is the same.

#### A.4.1 Threads vs Processes

> [!NOTE]
> We will learn more about the thread in [tutorial B](Week_10_Tutorial_B.md) this week. Please refer to there.

**Threads in the same process naturally share the process address space.** So if two threads access a global variable:

```c
int counter = 0;
```

both threads are accessing the same `counter`. That is why shared memory is commonly associated with threads.

**Processes do not normally share memory.**

After `fork()`, the child starts as a copy of the parent, but normal memory is not simply shared in the same way. Modern systems use copy-on-write, so the parent and child may initially refer to the same physical pages, but when one writes, the kernel gives it a private copy. So this does not work as shared communication:

```c
int x = 0;

pid_t pid = fork();

if (pid == 0) {
    x = 10;
    _exit(0);
}

wait(NULL);
printf("%d\n", x);
```

The parent will still usually print `0` because the child changed its own copy. To make processes truly share a region, we need a shared mapping.

#### A.4.2 `mmap()`

`mmap()` creates a mapping in a process’s virtual address space. A mapping means this range of virtual addresses refers to this underlying object or memory region. The general shape is:

```c
void *mmap(void *addr,
           size_t length,    // how large the mapping should be
           int prot,         // permissions, such as PROT_READ | PROT_WRITE
           int flags,        // MAP_SHARED or MAP_PRIVATE
           int fd,           // backing object, often from shm_open()
           off_t offset);    // where in the backing object to start
```

A useful mental model is:
- `mmap` does not just "allocate memory"
- `mmap` creates a relationship between virtual addresses and some backing storage

That backing storage might be a file, a POSIX shared memory object or anonymous memory.

#### A.4.3 `MAP_SHARED` vs `MAP_PRIVATE`

**`MAP_SHARED` means changes made through the mapping are visible to other processes that map the same underlying object.**

```c
mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
```

This is what we want for inter-process shared memory.

Conceptually:
- process A writes to `shared[0]`
- process B can read the changed value from its own mapping

**`MAP_PRIVATE` means changes are private to the process.**

This is often copy-on-write.

```c
mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
```

This does not provide normal shared-memory communication.

Conceptually:
- process A writes to its mapping
- process B does not necessarily see that change

So for shared memory between processes, use `MAP_SHARED` instead of `MAP_PRIVATE`.

#### A.4.4 Shared Memory between Related Processes

If processes are related through `fork()`, we can create an anonymous shared mapping before forking.

```c
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    int *shared = mmap(NULL,
                       sizeof *shared,
                       PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS,
                       -1,
                       0);

    if (shared == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    *shared = 0;

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        *shared = 42;
        _exit(0);
    }

    wait(NULL);

    printf("shared value = %d\n", *shared);

    munmap(shared, sizeof *shared);
    return 0;
}
```

Example output:

```text
shared value = 42
```

#### A.4.5 Shared Memory between Independent Processes

If the processes are independent, they need a way to find the same shared memory object.

That is what `shm_open()` provides. `shm_open()` creates or opens a named POSIX shared memory object. It returns a file descriptor, similar to `open()`. The usual pattern is:

```text
shm_open      create/open shared memory object
ftruncate     set its size
mmap          map it into this process
use memory
munmap        remove mapping from this process
close         close the file descriptor
shm_unlink    remove the shared memory name
```

A newly created shared memory object usually starts with size `0`. Before mapping it, we must set its size:

```c
ftruncate(fd, size);
```

If we forget this, the mapping may fail, or accessing it may cause problems because the object is not large enough.

##### Example: Writer and Reader Programs

> [!IMPORTANT]
> Refer to [`shared_memory_writer.c`](./Codes/shared_memory_writer.c), [`shared_memory_reader.c`](./Codes/shared_memory_reader.c), and [`shared_memory_cleanup.c`](./Codes/shared_memory_cleanup.c) for demonstrates the use of shared memory.

The writer process creates shared memory and writes a value into it, while the reader process opens the same shared memory object and reads the value.

Compile with:

```bash
gcc -std=c11 -Wall -Wextra -pedantic shared_memory_writer.c -o shared_memory_writer
gcc -std=c11 -Wall -Wextra -pedantic shared_memory_reader.c -o shared_memory_reader
gcc -std=c11 -Wall -Wextra -pedantic shared_memory_cleanup.c -o shared_memory_cleanup
```

and then run:

```bash
./shared_memory_writer
./shared_memory_reader
```

Example output should looks like:

```text
writer stored value 123
reader saw value 123
```

The two programs are independent processes, but they map the same shared memory object:

```text
/comp2017_shared_example
```

> [!WARNING]
> The shared memory object name can remain after the processes exit. To remove it, one process should call:
>
> ```c
> shm_unlink(SHM_NAME);
> ```

This removes the name. The actual object is destroyed once no process still has it open or mapped.

Run:

```bash
./shared_memory_cleanup
```

#### A.4.6 Sharing Data Does Not Synchronize Access

Shared memory only makes data visible. It does not automatically make access safe. For example, if two processes both do this:

```c
data->value++;
```

there is a race condition.

That operation is really several steps:

```text
read old value
add one
write new value
```

If two processes do it at the same time, one update can overwrite the other. So shared memory often needs synchronization, such as:

```text
POSIX semaphores
process-shared mutexes
file locks
pipes/signals for coordination
```

---

### A.5 Exercise: Word Count II

The goal is to search a large text file for a string using `mmap()`. Instead of repeatedly calling `read()` to copy file data into a buffer, we map the file into memory and then scan it like an array of characters.

> [!IMPORTANT]
> Refer to [`word_count2.c`](./Codes/word_count2.c) for the solution to this exercise.

We will search line by line. For each line:

```text
scan the line for the search string
if the line contains at least one match, print the line number and content
count every occurrence found in that line
```

This solution counts overlapping matches. For example, searching for `ana` inside `banana` will counts two occurrences.

```text
banana
 ^^^
   ^^^
```

If non-overlapping matches are wanted instead, the search loop can skip ahead by the search string length after a match.

#### A.5.1 Step 1: Open and Map the File

We first open the file:

```c
int fd = open(filename, O_RDONLY);
```

Then use `fstat()` to find its size:

```c
struct stat st;
fstat(fd, &st);
```

Then map it:

```c
char *data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
```

The mapping is read-only because we only need to search the file.

#### A.5.2 Step 2: Search Case-Insensitively

C strings are normally null-terminated, but an `mmap()` file is not guaranteed to have a `'\0'` at the end. So we should not use ordinary string functions like this on the mapped file:

```c
strstr(data, needle);
```

Instead, we compare characters manually using known lengths. For case-insensitive comparison:

```c
tolower((unsigned char)c)
```

The cast to `unsigned char` matters because `tolower()` is only safe for `EOF` or values representable as `unsigned char`.

#### A.5.3 Step 3: Print Matching Lines and Count Occurrences

The file is scanned line by line. A line starts at `line_start` and ends before `'\n'`.

If a line contains the search string, print:

```text
line_number: line content
```

The total occurrence count is printed at the end.

#### A.5.4 Notes

The file is mapped with:

```c
mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
```

After this, `data[i]` behaves like accessing bytes from the file.

The program does not use `strstr()` because the mapped file is not a normal C string. It is just a block of memory with a known size.

The search works by comparing the search string against each possible position in each line:

```c
if (match_at(data + pos, needle, needle_len)) {
    total++;
    line_has_match = 1;
}
```

The line is printed once if it has at least one match, but the total count increases for every occurrence.

### A.6 Semaphore

A **semaphore** is a synchronization tool used to control access to shared resources or to signal between processes/threads.

Think of a semaphore as a **counter**.

```text
sem_wait():
    If the counter is greater than 0, decrease it and continue.
    If the counter is 0, block and wait.

sem_post():
    Increase the counter.
    Wake up one waiting process/thread if there is one.
```

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

A semaphore can fix this by making the reader wait until the writer says:

```text
The data is ready.
```

#### A.6.1 Basic POSIX Semaphore Functions

For separate programs, such as:

```bash
./shared_memory_writer
./shared_memory_reader
```

you usually use a **named semaphore**.

Common functions:

```c
sem_open()    // create or open a named semaphore
sem_wait()    // wait / lock / block if not ready
sem_post()    // signal / unlock / wake someone
sem_close()   // close this process's reference
sem_unlink()  // remove the named semaphore from the system
```

You need:

```c
#include <semaphore.h>
```

and often:

```c
#include <fcntl.h>
```

#### A.6.2 Simple Writer Idea

The writer creates shared memory, writes data, then signals the reader:

```c
sem_t *sem = sem_open("/my_semaphore", O_CREAT, 0600, 0);

if (sem == SEM_FAILED) {
    perror("sem_open");
    return 1;
}

/* Write data into shared memory here. */

sem_post(sem);   // tell the reader: data is ready

sem_close(sem);
```

The initial semaphore value is `0`, meaning:

```text
Reader must wait.
Data is not ready yet.
```

After writing the data, the writer calls:

```c
sem_post(sem);
```

That changes the semaphore so the reader can continue.

#### A.6.3 Simple Reader Idea

The reader opens the same semaphore and waits:

```c
sem_t *sem = sem_open("/my_semaphore", 0);

if (sem == SEM_FAILED) {
    perror("sem_open");
    return 1;
}

sem_wait(sem);   // wait until writer says data is ready

/* Read data from shared memory here. */

sem_close(sem);
```

This line may block:

```c
sem_wait(sem);
```

It means:

```text
Do not continue until the writer posts the semaphore.
```

#### A.6.4 Important Idea

For shared memory:

```text
Shared memory = where the data is stored
Semaphore     = controls when it is safe to access the data
```

A simple pattern is:

```text
Reader waits.
Writer writes data.
Writer posts semaphore.
Reader wakes up.
Reader reads data.
```

#### A.6.5 Binary Semaphore

A semaphore used with values `0` and `1` is often called a **binary semaphore**.

```text
0 = not available / must wait
1 = available / can continue
```

It is similar to a lock, but semaphores can also be used for signaling between processes.

#### A.6.6 Counting Semaphore

A semaphore can also count more than one resource.

Example:

```text
Semaphore value = 3
```

This could mean:

```text
Three workers may enter.
The fourth worker must wait.
```

But for beginner shared memory examples, you usually use a binary-style semaphore.

#### A.6.7 Cleanup

Named semaphores should be removed when no longer needed:

```c
sem_unlink("/my_semaphore");
```

Usually one program, often the writer or a cleanup section, does this.

---

### A.7 Advanced Exercise: Real-time Stock Ticker

The publisher and subscriber are separate processes, so normal variables are not shared. Shared memory solves the storage problem:
- publisher writes latest stock prices into shared memory
- subscriber reads latest stock prices from shared memory

But shared memory does not solve the notification problem. The subscriber still needs to know: "when has the publisher written a new update?" So we need a second mechanism.

> [!NOTE]
> **Read sections start from A.7.2 and after your own attempt, you could go back to A.7.1 to see my sample solution.**

#### A.7.1 Codes and Usages

> [!IMPORTANT]
> Refer to the folder [`bash_simulator/*`](./Codes/bash_simulator/) for codes.

```text
stock_ticker/
├── stock_common.h
├── publisher.c
└── subscriber.c
```

Compile:

```bash
gcc -std=c11 -Wall -Wextra -pedantic publisher.c -o publisher -pthread
gcc -std=c11 -Wall -Wextra -pedantic subscriber.c -o subscriber -pthread
```

On some older Linux systems, if the linker complains about shared memory or semaphores, add:

```bash
-lrt
```

Run the publisher first:

```bash
./publisher
```

Then in another terminal, run:

```bash
./subscriber
```

**Example behavior**

Publisher:

```text
publisher: update 1: TSLA = 179.38
publisher: update 2: AAPL = 190.44
publisher: update 3: NVDA = 849.72
```

Subscriber:

```text
subscriber: update 1
  AAPL  190.00
  MSFT  410.00
  TSLA  179.38
  NVDA  850.00

subscriber: update 2
  AAPL  190.44
  MSFT  410.00
  TSLA  179.38
  NVDA  850.00
```

The exact prices will change because the publisher uses random updates.

#### A.7.2 How Does the Subscriber Know There Is an Update?

The shared memory contains the data, but it does not automatically wake the subscriber.

This field:

```c
int updated;
```

can tell us that the data changed, but only if the subscriber checks it.

If the subscriber does this:

```c
while (shared->updated == 0) {
    /* wait */
}
```

that is busy waiting. It wastes CPU.

So the better design is:

```text
shared memory stores the stock data
a semaphore or signal notifies the subscriber
a mutex/semaphore protects the shared data while it is being updated
```

#### A.7.3 Should We Use Signals?

Signals are possible, but they are not the cleanest choice here.

A signal such as `SIGUSR1` can mean:

```text
publisher tells subscriber: "there is new data"
```

But signals have limitations:

```text
the publisher must know the subscriber's PID
standard signals can coalesce, so multiple updates may become one notification
the signal handler should not print or read complex shared data directly
signals do not protect shared memory from races
```

So signals can be used as a wake-up mechanism, but they are not enough by themselves.

For this exercise, a named POSIX semaphore is cleaner:

```text
publisher updates shared memory
publisher posts an update semaphore
subscriber waits on the update semaphore
subscriber wakes and reads the latest data
```

This avoids polling and keeps the code easier to reason about.

#### A.7.4 Design Choice

We will use:

```text
shm_open + mmap       shared stock data
sem_open              named semaphores for independent processes
one mutex semaphore   protects the shared memory while reading/writing
one update semaphore  wakes the subscriber when new data is available
```

```c
typedef struct {
    char name[10];
    float price;
} Stock;

typedef struct {
    Stock stocks[MAX_STOCKS];
    int updated;
} SharedData;
```

Here, `updated` is used as an update counter, not just a boolean flag.

That is better because:

```text
updated = 0 means no updates yet
updated = 1 means first update
updated = 2 means second update
...
```

#### A.7.5 Notes

The shared memory object stores the current stock data:

```c
SharedData *data = mmap(..., MAP_SHARED, ...);
```

Because both programs map the same shared memory object, changes made by the publisher become visible to the subscriber.

The mutex semaphore protects access:

```text
publisher locks mutex before writing
subscriber locks mutex before reading
```

This prevents the subscriber from reading while the publisher is halfway through an update.

The update semaphore is the notification mechanism:

```text
subscriber blocks in sem_wait(update)
publisher calls sem_post(update)
subscriber wakes up
```

So the subscriber does not need to busy-wait.

#### A.7.6 What If We Used Signals Instead?

A signal-based design could work like this:

```text
subscriber stores its PID in shared memory
publisher updates shared memory
publisher sends SIGUSR1 to subscriber
subscriber wakes up and reads the latest data
```

But the signal handler should not directly print the stock table. A safer signal handler would only set a flag:

```c
static volatile sig_atomic_t got_update = 0;

static void handle_sigusr1(int sig) {
    (void)sig;
    got_update = 1;
}
```

Then the main loop would notice the flag and read the shared memory.

Signals are useful as notifications, but they do not replace locking. The shared data still needs protection while one process writes and another reads.

Also, standard signals are not a reliable "one signal per update" queue. If several updates happen quickly, the subscriber may only observe that "something changed" and then read the latest state.

So for this exercise:

```text
signals are possible
semaphores are simpler and safer
```

#### A.7.7 Common Mistakes

A common mistake is using `updated` as if it automatically wakes the subscriber:

```c
while (!data->updated) {
}
```

That is busy waiting and wastes CPU.

Another mistake is sharing memory without synchronization:

```c
data->stocks[i].price = new_price;
```

while the subscriber reads at the same time. The subscriber may see partially updated or inconsistent data.

Also, `shm_open()` only creates or opens the shared memory object. We still need:

```text
ftruncate() to set its size
mmap() to map it into memory
```

Finally, remember that:

```text
shared memory stores data
semaphores or signals notify/synchronise access
```

They solve different parts of the problem.

---

### A.7 Exercise: `poll` / `epoll` Monitoring Children

We want one parent process to monitor output from five child processes.

Each child has its own pipe:

```text
child stdout/stderr -> pipe write end
parent reads        <- pipe read end
```

The parent should not read pipes one by one with blocking `read()`, because one quiet child could stall the whole logger.

Instead, the parent waits until any pipe is readable using either:

```text
poll()
epoll()
```

After the parent reads a child’s message, it sends that child `SIGUSR1`, allowing it to exit.

The child redirects:

```text
stdin  -> /dev/null
stdout -> pipe write end
stderr -> pipe write end
```

So anything the child prints using `printf()` or `fprintf(stderr, ...)` goes to the parent.

The parent keeps only the read end of each pipe.

For this exercise, the parent uses a blocking `read()` only after `poll()` or `epoll()` says the pipe is ready. That is okay here because each child writes one short message. In larger event-loop programs, it is usually better to make the read fds non-blocking and read until `EAGAIN`.

> [!IMPORTANT]
> Refer to [`monitor.c`](./Codes/monitor.c) for the solution to this exercise.

> [!NOTE]
> The scaffold uses:
>
> ```c
> static volatile int got_sigusr1 = 0;
> ```
>
> A better signal-handler flag type is:
>
> ```c
> static volatile sig_atomic_t got_sigusr1 = 0;
> ```
>
> This is the standard type intended for simple values shared between normal code and a signal handler.

#### A.7.1 `poll` Version

The `poll` version stores all pipe read ends in an array:

```c
struct pollfd fds[N_CHILDREN];
```

Each entry says:

```text
watch this fd for readable data
```

The parent waits with:

```c
poll(fds, N_CHILDREN, -1);
```

After `poll()` returns, the parent scans the whole array and checks `fds[i].revents`.

So the important idea is:

```text
poll waits for readiness, but I still scan the array afterwards
```

#### A.7.2 `epoll` Version

The `epoll` version first creates an epoll instance:

```c
int epfd = epoll_create1(0);
```

Then it registers each pipe read end:

```c
epoll_ctl(epfd, EPOLL_CTL_ADD, pipes[i][0], &ev);
```

The parent waits with:

```c
epoll_wait(epfd, events, N_CHILDREN, -1);
```

The important difference is that `epoll_wait()` gives back only the ready events.

So the important idea is:

```text
epoll stores the watched fd set in the kernel
epoll_wait returns the fds that are ready
```

#### A.7.3 `pause` Note

This exercise asks for:

```c
while (!got_sigusr1) {
    pause();
}
```

That is fine for demonstrating the idea, but it has the known `pause()` race: if the signal arrives after the condition is checked but before `pause()` starts sleeping, the child can block forever.

A more robust version would block `SIGUSR1` first and wait using `sigsuspend()` or `sigwait()`. For this exercise, the main focus is the parent-side difference between `poll()` and `epoll()`.

#### A.7.4  Short Summary

The parent creates one pipe per child. Each child redirects `stdin` to `/dev/null` and redirects both `stdout` and `stderr` to its pipe.

The parent monitors all pipe read ends in one thread.

With `poll()`, the parent passes an array of fds each time and scans it after readiness is reported.

With `epoll()`, the parent registers fds once, and `epoll_wait()` returns only ready events.

Both approaches avoid the bad blocking pattern where one quiet child can stall output from every other child.
