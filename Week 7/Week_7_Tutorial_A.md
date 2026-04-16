## COMP2017 2026 S1 Week 7 Tutorial A

<table><tbody>
  <tr><td><b>Tutor</b></td><td>Hao Ren</td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au">hao.ren@sydney.edu.au</a></td></tr>
</tbody></table>

- [COMP2017 2026 S1 Week 7 Tutorial A](#comp2017-2026-s1-week-7-tutorial-a)
  - [A.1 Exercise: Find Max (through Function Pointer)](#a1-exercise-find-max-through-function-pointer)
    - [A.1.1 Implement the `*_value` Functions](#a11-implement-the-_value-functions)
    - [A.1.2 Implement `max()`](#a12-implement-max)
  - [A.2 Exercise: Filtering Linked List](#a2-exercise-filtering-linked-list)
  - [A.3 Signals](#a3-signals)
    - [A.3.1 Process Identifier (PID)](#a31-process-identifier-pid)
    - [A.3.2 Sending Signals From the Shell](#a32-sending-signals-from-the-shell)
    - [A.3.2 What Happens When a Signal Arrives](#a32-what-happens-when-a-signal-arrives)
    - [A.3.3 `signal()` vs `sigaction()`](#a33-signal-vs-sigaction)
      - [Why `sigaction()` is Preferred?](#why-sigaction-is-preferred)
    - [A.3.4 Signal Safety: The Most Important Practical Rule](#a34-signal-safety-the-most-important-practical-rule)
    - [A.3.5 The Recommended Pattern: Set a Flag And Let Main Code Do The Work](#a35-the-recommended-pattern-set-a-flag-and-let-main-code-do-the-work)
  - [A.4 Exercise: Protected Calculator](#a4-exercise-protected-calculator)
  - [A.5 Exercise: Hard to Kill](#a5-exercise-hard-to-kill)

---

### A.1 Exercise: Find Max (through Function Pointer)

> [!IMPORTANT]
> Refer to [`max.c`](./Codes/max.c) for the solution to this exercise.

This `max` function is meant to be **generic**.

That means it should work for many different array element types:

- `int`
- `char *`
- `struct item`

Because `max` does not know the real element type, it receives:

- `void *base` — where the array starts
- `size_t n` — how many elements there are
- `size_t size` — how big each element is
- `int (*value)(const void *)` — a function that tells us the "value" of one element

So the key idea is that `max` does not compare the elements directly. It asks the callback function `value()` what numeric value each element has, then keeps the element with the largest value.

#### A.1.1 Implement the `*_value` Functions

These functions each receive a pointer to **one element** of the array.

If the array element is an `int`, then `a` points to an `int`. So we cast `a` to `const int *`, then dereference it.

```c
int int_value(const void *a) {
    return *(const int *)a;
}
```

For `string_length_value()`, Each element is a `char *`.

```c
char *words[] = {"apple", "hippopotamus", "cat", "banana"};
```

So inside `max`, when we look at one element of this array, we get a pointer to that element. That means `a` is really pointing to a `char *`, so its true type is `const char * const *`.

```c
int string_length_value(const void *a) {
    return (int)strlen(*(const char * const *)a);
}
```

Moreover, if the element is a `struct item`, then `a` points to one `struct item`. So cast it and return its `count` field.

```c
int struct_count_value(const void *a) {
    return ((const struct item *)a)->count;
}
```

#### A.1.2 Implement `max()`

We want to walk through the array element by element. Since `base` is a `void *`, we cannot do normal array indexing directly on it in standard C. So we first cast it to `char *`, because `char *` moves one byte at a time, so:

```c
bytes + i * size
```

moves to the `i`th element. Then the algorithm is:

1. assume the first element is the current maximum
2. loop through the rest of the array
3. call `value(current)` for each element
4. if the current value is larger, update the maximum pointer

---

### A.2 Exercise: Filtering Linked List

> [!IMPORTANT]
> Refer to [`ship.c`](./Codes/ship.c) for the solution to this exercise.

A predicate is just a function that answers "keep this node or not?" `filter_ships` should not hard-code one filtering rule. Instead, it should take a predicate function and keep only the ships that satisfy that rule.

In this scaffold, the ships are local variables in `main`, not heap-allocated nodes. So removing a ship means **unlinking it from the list**, not calling `free`.

```c
typedef int (*ship_predicate)(const struct ship *);

int has_enough_fuel(const struct ship *s) {
    return s->fuel >= 50;
}

int has_enough_distance(const struct ship *s) {
    return s->distance >= 100.0f;
}
```

The function first removes any failing nodes at the front of the list, because the head itself may need to change. It then walks through the rest of the list and removes a node by skipping it:

```c
curr->next = curr->next->next;
```

This makes the filtering logic reusable. The list traversal stays the same, and only the predicate changes.

The second call:

```c
fleet = filter_ships(fleet, has_enough_distance);
```

runs on the already-filtered list, not the original one.

---

### A.3 Signals

A **signal** is an asynchronous notification sent to a running process. Each signal has a **default action**, such as terminate, stop, continue, ignore, or terminate-and-core-dump. A process can keep the default behavior, ignore the signal, or install a **signal handler** to catch it. On Linux/POSIX systems, `sigaction()` and `signal()` are the interfaces used to change a signal's disposition, but `sigaction()` is the preferred one.

These are the most useful signals: `SIGINT` means "interrupt," and its default action is to terminate the process. This is what users usually trigger with `Ctrl+C`. `SIGTERM` is the normal termination request; it also defaults to terminating the process, but programs are allowed to catch it and clean up first. `SIGKILL` forces termination and cannot be caught, blocked, or ignored. `SIGTSTP` is the stop signal typed at the terminal, usually from `Ctrl+Z`, while `SIGSTOP` is the uncatchable stop signal. `SIGCONT` resumes a stopped process.

- `SIGINT`: "please interrupt"
- `SIGTERM`: "please terminate cleanly"
- `SIGKILL`: "die immediately"
- `SIGTSTP`: "pause because the user typed stop"
- `SIGSTOP`: "hard stop from the system or another process"
- `SIGCONT`: "continue running"

#### A.3.1 Process Identifier (PID)

- `ps`
- `top` / `htop`
- `pgrep`
- `pidof`

#### A.3.2 Sending Signals From the Shell

The `kill` command is unfortunately named: it does **not** only send "kill" signals. It sends **any** signal. If you omit the signal, `kill` sends `SIGTERM` by default, and the manual explicitly recommends trying `TERM` before `KILL`, because `TERM` gives the process a chance to clean up.

Examples:

```bash
kill -TERM 255
kill -KILL 255
kill -SIGINT 255
```

You can also use signal numbers, though names are usually clearer:

```bash
kill -15 255   # TERM
kill -9 255    # KILL
kill -2 255    # INT
```

There is also a useful special case:

```bash
kill -0 255
```

This sends no real signal, but it still performs error checking, so it is often used to test whether a process exists and whether you have permission to signal it.

#### A.3.2 What Happens When a Signal Arrives

Signals are asynchronous, which means they can arrive while your program is doing almost anything. When a signal is delivered and your program has installed a handler, the kernel interrupts normal execution, sets up the handler call, and then resumes normal execution after the handler returns. The signal handler runs with the signal's disposition already chosen, and unless you request otherwise, the signal that triggered the handler is blocked while that handler is running.

Another subtle but useful fact is that ordinary "standard" signals are **not queued**. If the same standard signal is generated multiple times while it is blocked, usually only one pending instance is remembered. That is one reason signal handlers should stay short: signals are not a general-purpose message queue.

#### A.3.3 `signal()` vs `sigaction()`

`signal()` is not gone; it still exists. The real issue is that using `signal()` to install a handler has historically had different semantics on different systems, and the Linux man page says the only portable use of `signal()` is to set a disposition to `SIG_DFL` or `SIG_IGN`. POSIX solved that portability mess by standardizing `sigaction()` and recommends using `sigaction()` instead.

> [!IMPORTANT]
> Refer to [`signal_interrupt.c`](./Codes/signal_interrupt.c) for the code used as demonstration.

The important ideas are that the handler takes one `int` argument, the signal number, and that the handler uses `write()` instead of `printf()`, because `write()` is async-signal-safe and `printf()` is not. The problem with `signal()` here is portability and semantics, not the one-argument handler shape itself.

##### Why `sigaction()` is Preferred?

`sigaction()` gives you explicit control over the signal behavior. It lets you choose between the simple one-argument handler (`sa_handler`) and the extended three-argument handler (`sa_sigaction`), specify which signals should be blocked while the handler runs (`sa_mask`), and choose behavior flags in `sa_flags`, such as `SA_SIGINFO`, `SA_RESTART`, and `SA_NODEFER`. `sigaction()` can be used for any signal except `SIGKILL` and `SIGSTOP`.

- `sa_handler`: simple handler, `void handler(int)`
- `sa_sigaction`: extended handler, `void handler(int, siginfo_t *, void *)`
- `sa_mask`: signals to block during the handler
- `sa_flags`: option bits such as `SA_SIGINFO` and `SA_RESTART`

A cleaner setup than `memset` is to use `sigemptyset()` for the mask, because `sigemptyset()` explicitly initializes a signal set so that no signals are included.

> [!IMPORTANT]
> Refer to [`sigaction_interrupt.c`](./Codes/sigaction_interrupt.c) for the code used as demonstration.

With `SA_SIGINFO`, the handler gets extra information through the `siginfo_t *` argument. That is useful if you want to inspect who sent the signal, or, for some signals, additional delivery details. Unless you set `SA_NODEFER`, the signal that triggered the handler is automatically blocked while the handler runs.

#### A.3.4 Signal Safety: The Most Important Practical Rule

The most important practical rule is this: **Inside a signal handler, only call async-signal-safe functions.**

The `signal-safety(7)` man page says many functions are not async-signal-safe, especially nonreentrant ones, and it explicitly uses `printf()` as the classic example of what can go wrong. Stdio functions maintain internal buffers and bookkeeping state; if a signal interrupts `printf()` and the handler calls `printf()` again, the second call may operate on inconsistent internal state and produce undefined behavior. That is why `printf`, `fprintf`, `fgets`, `malloc`, `realloc`, and `free` are all bad choices inside a signal handler.

This is why our examples often use `write()` instead. `write()` is on the async-signal-safe list, and it writes directly to a file descriptor without stdio buffering.

#### A.3.5 The Recommended Pattern: Set a Flag And Let Main Code Do The Work

The cleanest pattern for real programs is to keep the handler tiny. Usually the handler should do one of two things:

- write a short fixed message with `write()`, or
- set a flag that the main loop checks later.

For the flag pattern, `sig_atomic_t` is the right type to know. POSIX defines `sig_atomic_t` as an integer type that can be accessed as an atomic entity even in the presence of asynchronous interrupts.

For example:

```c
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

volatile sig_atomic_t got_sigint = 0;

void handler(int sig) {
    (void)sig;
    got_sigint = 1;
}

int main(void) {
    struct sigaction sa = {0};
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    while (!got_sigint) {
        pause();
    }

    printf("Cleaning up safely in normal code\n");
    return 0;
}
```

The important point is that the unsafe work, such as `printf()`, happens **after** the handler returns, in ordinary program flow, not inside the handler itself.

`SA_SIGINFO` switches from `sa_handler` to `sa_sigaction` and gives the handler extra information. `SA_RESTART` asks the system to automatically restart certain interrupted system calls. `SA_NODEFER` prevents the delivered signal from being automatically blocked while its handler runs. These are all part of why `sigaction()` is more powerful than `signal()`.

For a beginners, `SA_SIGINFO` is the most important one to introduce first, because it explains why the handler signature changes from one argument to three. `SA_RESTART` is the next most useful flag to mention, because signals can interrupt blocking system calls.

---

### A.4 Exercise: Protected Calculator

The signal you want is might be `SIGFPE`: on typical Unix/Linux systems, integer division by zero may generate `SIGFPE`, although the C standard itself treats integer divide-by-zero as undefined behavior. `sigaction()` is the standard way to install the handler, and `write()` plus `_exit()` are both async-signal-safe, which makes them suitable inside the handler. `printf()`, `fprintf()`, `perror()`, and `exit()` are not the right choice inside the handler.

> [!IMPORTANT]
> Refer to [`calculator.c`](./Codes/calculator.c) for the code used in this section.

---

### A.5 Exercise: Hard to Kill

`Ctrl+C`, `Ctrl+Z`, and `Ctrl+\` correspond to `SIGINT`, `SIGTSTP`, and `SIGQUIT`. All three can be caught with `sigaction()`, while `SIGKILL` and `SIGSTOP` cannot. For the Fibonacci timing, `alarm(5)` is a good fit because it schedules a `SIGALRM` for 5 seconds later, and `pause()` then lets the process sleep until some signal arrives.

Inside signal handlers, you should keep the work minimal. `signal-safety(7)` explicitly says stdio functions are not async-signal-safe, while `write(2)` and `_exit(2)` are safe. That is why the handlers below only update `sig_atomic_t` flags, print short fixed messages with `write()`, and use `_exit()` on the final correct `Ctrl+\`. POSIX defines `sig_atomic_t` as a type that can be accessed atomically even with asynchronous interrupts.

> [!IMPORTANT]
> Refer to [`hard_to_kill.c`](./Codes/hard_to_kill.c) for the code used in this section.

The program keeps one global `stage` variable. `stage == 0` means no correct prefix has been entered yet, `stage == 1` means `Ctrl+C` was received first, and `stage == 2` means `Ctrl+C` then `Ctrl+Z` were received in the right order. Any wrong signal resets `stage` back to `0` and prints `Kill Sequence Aborted`. The final correct `Ctrl+\` prints the completion message and exits immediately from the handler. That matches the required sequence logic.

The Fibonacci printing is separated from the kill-sequence logic. `SIGALRM` does not print directly; it only sets the `tick` flag. The main loop wakes up from `pause()`, notices `tick`, prints the next Fibonacci number, advances the sequence, and arms another 5-second alarm. This keeps the handlers simple and avoids putting unnecessary logic inside them. `pause()` is specifically designed to suspend execution until a signal is caught, and `alarm()` specifically schedules delivery of `SIGALRM`. ([man7.org][3])

The `sa_mask` part is useful here too. `sigaction()` lets you specify a set of signals that should be blocked while the handler runs, and the delivered signal is also blocked by default unless `SA_NODEFER` is used. Blocking `SIGINT`, `SIGTSTP`, `SIGQUIT`, and `SIGALRM` during the handler helps avoid overlapping handler executions while `stage` is being updated and a message is being written.

The code also used `alarm()` plus `pause()` rather than `sleep(5)`. That makes the whole program event-driven around signals. It also avoids mixing `alarm()` with `sleep()`, which `alarm(2)` warns is a bad idea because `sleep()` may itself be implemented using `SIGALRM`.
