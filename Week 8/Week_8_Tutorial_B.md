## COMP2017 2026 S1 Week 8 Tutorial B

<table><tbody>
  <tr><td><b>Tutor</b></td><td>Hao Ren</td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au">hao.ren@sydney.edu.au</a></td></tr>
</tbody></table>

- [COMP2017 2026 S1 Week 8 Tutorial B](#comp2017-2026-s1-week-8-tutorial-b)
  - [B.1 Process](#b1-process)
    - [B.1.1 Typical Process Memory Layout](#b11-typical-process-memory-layout)
    - [B.1.2 inspecting the Executable versus Inspecting the Running Process](#b12-inspecting-the-executable-versus-inspecting-the-running-process)
    - [B.1.3 Reading the Sample Disassembly Line](#b13-reading-the-sample-disassembly-line)
  - [B.2 Race Condition](#b2-race-condition)
    - [B.2.1 Data-related Race Conditions](#b21-data-related-race-conditions)
    - [B.2.2 Event-related Race Conditions](#b22-event-related-race-conditions)
    - [B.2.3 Examples](#b23-examples)
      - [Example 1: Bank Account Update](#example-1-bank-account-update)
      - [Example 2: Shared Counter](#example-2-shared-counter)
      - [Example 3: Two Processes Writing to the Same File](#example-3-two-processes-writing-to-the-same-file)
      - [Example 4: Signal and `pause()`](#example-4-signal-and-pause)
      - [Example 5: Reading Shared Memory Too Early](#example-5-reading-shared-memory-too-early)
  - [B.3 `fork()`](#b3-fork)
    - [B.3.1 `fork()` and Memory: What is Copied, and What is Shared](#b31-fork-and-memory-what-is-copied-and-what-is-shared)
    - [B.3.2 `fork()`, `fork()`, and `fork()`!](#b32-fork-fork-and-fork)
  - [B.4 `wait()`](#b4-wait)
    - [B.4.1 `wait()` versus `waitpid()`](#b41-wait-versus-waitpid)
    - [B.4.2 Why `wait()` Matters: Zombie Processes](#b42-why-wait-matters-zombie-processes)
    - [B.4.3 Blocking Behavior](#b43-blocking-behavior)
    - [B.4.4 The Non-blocking Flag: `WNOHANG`](#b44-the-non-blocking-flag-wnohang)
    - [B.4.5 `wait()` with `sleep()` and `wait()` with `fork()`](#b45-wait-with-sleep-and-wait-with-fork)
    - [B.4.6 Getting the child's Exit Status](#b46-getting-the-childs-exit-status)
    - [B.4.7 Other `waitpid()` Options](#b47-other-waitpid-options)
  - [B.5 `exec*()`](#b5-exec)
    - [B.5.1 The Scaffold Example](#b51-the-scaffold-example)
    - [B.5.2 How Shells Launch Commands](#b52-how-shells-launch-commands)
    - [B.5.3 The `exec` Family](#b53-the-exec-family)
      - [`l` Means "List"](#l-means-list)
      - [`v` Means "Vector"](#v-means-vector)
      - [`p` Means "Search PATH"](#p-means-search-path)
      - [`e` Means "Explicit Environment"](#e-means-explicit-environment)
      - [`argv` And `argv[0]`](#argv-and-argv0)
      - [`execve()` is the Underlying Form](#execve-is-the-underlying-form)
    - [B.5.4 What Stays the Same Across `exec` and What Not](#b54-what-stays-the-same-across-exec-and-what-not)
  - [B.6 Exercise: Bash Simulator](#b6-exercise-bash-simulator)
    - [B.6.1 Read One Line and Split It into Arguments](#b61-read-one-line-and-split-it-into-arguments)
    - [B.6.2 Handle `exit` inside the Shell Itself](#b62-handle-exit-inside-the-shell-itself)
    - [B.6.3 Next Step: `fork()`](#b63-next-step-fork)
    - [B.6.4 Then Child Uses `execvp()`](#b64-then-child-uses-execvp)
    - [B.6.5 Meanwhile Parent Waits](#b65-meanwhile-parent-waits)

---

### B.1 Process

A process is the running instance of a program. What matters most for this topic is that a process has its own **memory image**, meaning its own virtual address space and its own currently mapped memory regions. It is the process's live in-memory view of the program while it runs. That includes executable code, global/static storage, heap allocations, the stack, mapped files/libraries, and the argument/environment data placed on the initial stack when a new program starts.

#### B.1.1 Typical Process Memory Layout

A simplified process layout is usually shown like this, from **higher** addresses to lower **addresses**:

- arguments and environment data on the initial stack
- **stack**, which typically grows downward
- memory-mapped regions, such as shared libraries and mapped files
- **heap**, which grows upward
- uninitialized globals and statics (`.bss`)
- initialized globals and statics (`.data`)
- program text (`.text`)

This is a useful model, but it is still a simplification. The exact mappings for a real running process are whatever the kernel places in that process's address space, and `/proc/<pid>/maps` is the place to inspect the actual result.

- The **stack** holds function call frames, local variables with automatic storage duration, return addresses, and related call state.
- The mapped-region area is where the process may have shared libraries, memory-mapped files, and other mappings.
- The **heap** is where dynamic allocations such as `malloc()` usually come from.
- The `.bss` region stores global and static variables that start out zero-initialized.
- The `.data` region stores global and static variables that have an explicit initial value.
- The **text** region is where the program's machine instructions live. This is the code that the CPU executes.

A short example helps:

```c
#include <stdio.h>
#include <stdlib.h>

int global_init = 123;      // usually in .data
int global_bss;             // usually in .bss
static int static_init = 7; // usually in .data
static int static_bss;      // usually in .bss

int main(int argc, char *argv[]) {
    int local = 5;                    // usually on the stack
    int *heap = malloc(sizeof *heap); // points into the heap

    printf("&main         = %p\n", (void *)&main);
    printf("&global_init  = %p\n", (void *)&global_init);
    printf("&global_bss   = %p\n", (void *)&global_bss);
    printf("&static_init  = %p\n", (void *)&static_init);
    printf("&static_bss   = %p\n", (void *)&static_bss);
    printf("&local        = %p\n", (void *)&local);
    printf("heap          = %p\n", (void *)heap);
    printf("argv          = %p\n", (void *)argv);
    printf("argv[0]       = %p -> %s\n", (void *)argv[0], argv[0]);

    free(heap);
    return 0;
}
```

The exact addresses are not the important part. The important part is the pattern: code has an address too, globals/statics live in their own long-lived regions, stack locals are separate from heap objects, and the argument vector is also part of the process's memory image.

#### B.1.2 inspecting the Executable versus Inspecting the Running Process

There are two different levels to inspect.

`readelf` inspects the **ELF file** on disk. If we want section headers such as `.text`, `.data`, and `.bss`, the usual command is:

```bash
readelf -S ./my_program
```

or equivalently:

```bash
readelf --section-headers ./my_program
```

The `readelf` documentation says `-S`, `--sections`, and `--section-headers` display the file's section headers. This is about the executable file format, not the live runtime address space.

`objdump` can disassemble the machine code inside the executable or object file. For code sections only, `-d` is the usual choice. For all non-empty non-BSS sections, `-D` disassembles more broadly. The `-M intel` option switches x86-family output from AT&T syntax to Intel syntax. So a useful command is:

```bash
objdump -d -M intel ./my_program
```

or, if we really want the broader disassembly:

```bash
objdump -D -M intel ./my_program
```

The `objdump(1)` page says `-d` disassembles instruction sections, `-D` disassembles all non-empty non-BSS sections, and `intel`/`att` choose the syntax flavor.

If the goal is to inspect the **actual live mappings** of a running process instead of just the ELF file on disk, then `/proc/<pid>/maps` is the more direct runtime view. For the current process, that usually means:

```bash
cat /proc/$$/maps
```

in a shell, or from inside a program we can inspect `/proc/self/maps`. The `proc_pid_maps(5)` page says this file contains the currently mapped memory regions and their access permissions.

For example, if we compile this program:

```c
int global_init = 1;
int global_bss;

int main(void) {
    return global_init + global_bss;
}
```

then:

```bash
readelf -S ./a.out
```

shows file sections such as `.text`, `.data`, and `.bss`, because those are part of the ELF structure on disk. But:

```bash
cat /proc/<pid>/maps
```

shows runtime mappings such as the executable mappings, shared libraries, stack, and heap, because that file is about the running process's memory image, not just the file layout. Those are related ideas, but not exactly the same thing.

#### B.1.3 Reading the Sample Disassembly Line

A line like this:

```text
40050d: c7 45 fc 04 00 00 00   mov DWORD PTR [rbp-0x4],0x4
```

is an instruction from the disassembly of the program's text section. `objdump` is reconstructing assembly mnemonics from the machine code bytes in the executable. In this example, the instruction is moving the constant `4` into the stack slot at offset `-0x4` from `rbp`, which is the kind of pattern commonly used for a local variable in a function prologue/body on x86-64. For now, the point is not mastering every assembly instruction, but recognizing that `.text` is the executable code region and `objdump` lets we inspect it.

---

### B.2 Race Condition

The core idea of race condition is: *A race condition happens when the program's result depends on the **timing** or **order** of events, and that order is not guaranteed.* So the problem is not just two things happen at once. The real problem is two or more execution paths depend on shared state or shared events, but there is no proper control over who goes first

Because of that, the program may:

- sometimes behave correctly
- sometimes behave incorrectly
- sometimes fail in strange ways that are hard to reproduce

That is why race conditions are difficult to debug.

Once multiple processes or threads can run independently, we **cannot** assume:

- process A runs before process B
- thread A finishes before thread B
- event A happens before event B
- a signal arrives after we are ready for it

The scheduler, system load, timing, interrupts, and hardware all affect the actual order.

#### B.2.1 Data-related Race Conditions

These happen when multiple execution paths access the same data, and at least one of them modifies it.

The issue is usually:

- read old value
- compute new value
- write new value

If two execution paths do this at the same time, one update can overwrite the other.

#### B.2.2 Event-related Race Conditions

These happen when correctness depends on events happening in the right order.

The issue is usually:

- I expected A to happen before B
- but B can happen before A
- so the program misses something or waits forever

Examples include:

- signal arrives before handler setup
- `pause()` happens after the signal already arrived
- reading shared memory before valid data is written

#### B.2.3 Examples

##### Example 1: Bank Account Update

Suppose Jon Snow has `$100` in his bank account.

Two separate workers both try to withdraw `$30`.

If each worker does this:

1. read current balance
2. subtract 30
3. write new balance

then the intended final answer should be:

```text
100 - 30 - 30 = 40
```

But this can go wrong.

If we have

1. Worker A reads `100`
2. Worker B reads `100`
3. Worker A writes `70`
4. Worker B writes `70`

Our final balance would be `70` which is **incorrect**. One withdrawal has effectively been lost.

##### Example 2: Shared Counter

Suppose two threads both try to do:

```c
counter++;
```

At first glance, this looks simple, but `counter++` is not one magical atomic action. It is more like:

1. read counter
2. add 1
3. write back

If `counter` starts at `0`, and two threads both increment it, we expect the result to be `2`.

But this can happen:

1. Thread A reads `0`
2. Thread B reads `0`
3. Thread A writes `1`
4. Thread B writes `1`

Our final value would be `1` instead of `2`. This is one of the classic race-condition examples.

##### Example 3: Two Processes Writing to the Same File

Suppose two processes both write log messages to the same file at the same time.

Process A wants to write:

```text
START_A
```

Process B wants to write:

```text
START_B
```

If file access is not coordinated, the log file may end up with:

```text
START_A
START_B
```

or:

```text
START_B
START_A
```

or in some cases badly interleaved output depending on how writes are done. So if the correctness of the file content depends on a specific order, then simultaneous writes can create a race condition. This is especially important for:

- logs
- databases
- shared configuration files
- shared output files

##### Example 4: Signal and `pause()`

Suppose the program logic is:

1. wait for a signal
2. when the signal arrives, continue

A naive version might do:

```c
signal_arrived = 0;

/* expect a signal here */

if (!signal_arrived) {
    pause();
}
```

The problem is:

- the signal might arrive **after** the `if` check
- but **before** the `pause()`

Then the signal handler sets the flag, returns, and after that the program calls `pause()`. Now there may be no more signals coming, so the program can block forever.

This program assumes:

```text
check flag -> then wait
```

but the signal can happen in the tiny gap between those two steps. So correctness depends on timing that is not controlled.

##### Example 5: Reading Shared Memory Too Early

Suppose process A writes some data into shared memory, and process B reads it.

Process A:

```text
write data
set ready flag
```

Process B:

```text
if ready flag is set, read data
```

This is fine only if the ordering is enforced correctly.

But if B reads before A has finished writing valid data, then B may see:

- stale data
- partially written data
- uninitialized data

Again, the bug is caused by wrong event ordering.

---

### B.3 `fork()`

`fork()` duplicates the current process.

After `fork()`, there are now **two** processes: the **parent** and the **child**.

Both continue executing from the line **after** the `fork()` call. That is the part that usually feels strange at first: `fork()` is called once, but after it returns, two processes are running. On success, `fork()` returns the child PID in the parent, `0` in the child, and `-1` on failure. The parent and child run in separate memory spaces that initially have the same contents.

The most important thing to remember is:

> parent and child start with the same memory contents, but they are different processes.

So after `fork()`:

- both have their own variables
- both have their own stack/heap/data memory space
- changing a variable in one process does not change it in the other

Linux implements this efficiently with **copy-on-write**. That means the child does not immediately get a full physical copy of all memory pages; instead, pages are copied only when one process writes to them.

```c
#include <stdio.h>
#include <unistd.h>

int main(void) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("I am the child\n");
    } else {
        printf("I am the parent, child pid = %d\n", pid);
    }

    return 0;
}
```

Possible output:

```text
I am the parent, child pid = 12345
I am the child
```

or:

```text
I am the child
I am the parent, child pid = 12345
```

The order is not guaranteed, because the scheduler decides which process runs first after the fork. Both processes continue from the same point after the call.

#### B.3.1 `fork()` and Memory: What is Copied, and What is Shared

**Separate Process Memory**

The child gets a separate memory space with the same initial contents. So variables like:

- stack variables
- heap memory
- global/static data

all begin with the same values, but later writes do not affect the other process.

**Open File Descriptors are Inherited**

The child also inherits copies of the parent's open file descriptors, and the corresponding descriptors refer to the **same open file description**, meaning they share things like file offset and status flags.

That is why both parent and child can write to the same standard output. It is also why file output after `fork()` can interleave unless the program coordinates it.

#### B.3.2 `fork()`, `fork()`, and `fork()`!

If we run the following code block:

```c
fork();
fork();
fork();
```

**8** subprocesses would be there, instead of 3 or 4.

---

### B.4 `wait()`

This section goes together with `fork()`, because once a parent creates a child, the parent often needs a way to:

- pause until the child finishes
- collect the child's exit status
- clean up the child's process-table entry

That is exactly what `wait()` and `waitpid()` are for.

> [!IMPORTANT]
> `wait()` and `waitpid()` do **not** wait for "another process" in general.
> They wait for a **child** of the calling process.

More precisely, `wait()` waits until one of the caller's children terminates, and `waitpid()` waits for a child selected by its `pid` argument to change state. By default, `waitpid()` waits for terminated children, but options can change that behavior.

After a `fork()`, the parent and child run independently. If we do nothing, the parent may finish first, the child may finish first, or their output may interleave in different ways. If we want the parent to pause until the child is done, we use `wait()` or `waitpid()`. If the child has already changed state, the call returns immediately; otherwise it blocks until a child changes state or a signal interrupts the call.

```c
pid_t pid = fork();

if (pid == 0) {
    /* child */
    return 0;
} else {
    /* parent */
    wait(NULL);
}
```

This makes the parent stop and wait until one child terminates. In fact, the man page states that:

```c
wait(&status)
```

is equivalent to:

```c
waitpid(-1, &status, 0)
```

So `wait()` is really the simple "wait for any child" form.

On success, `wait()` returns the PID of the terminated child. On failure, it returns `-1`. So this:

```c
pid_t done = wait(NULL);
printf("Child %d finished\n", done);
```

prints the PID of the child that was reaped. `waitpid()` also returns the PID of the child whose state changed; if `WNOHANG` is used and no matching child has changed state yet, then it returns `0` instead of blocking.

#### B.4.1 `wait()` versus `waitpid()`

`wait()` is the simple form:

```c
wait(NULL);
```

This means *wait for **one** terminated child, whichever one finishes first*. It is convenient if we do not care which child finishes first.

`waitpid()` is the more flexible form:

```c
waitpid(pid, NULL, 0);
```

This means *wait for the child with that specific PID*.

So the main difference is:

- `wait()` = implicitly wait for any one child
- `waitpid()` = explicitly choose which child or group of children to wait for

The `pid` argument to `waitpid()` can mean several things:

- `pid > 0` → wait for the child with exactly that PID
- `pid == -1` → wait for any child
- `pid == 0` → wait for any child in the same process group
- `pid < -1` → wait for any child whose process-group ID matches `abs(pid)`

So in a simple `fork()` example, these two are equivalent:

```c
wait(NULL);
```

```c
waitpid(-1, NULL, 0);
```

#### B.4.2 Why `wait()` Matters: Zombie Processes

When a child process terminates, it does not disappear completely right away. If the parent has not yet waited for it, the child becomes a **zombie**. A zombie is a terminated child for which the kernel still keeps a small amount of information, such as PID and termination status, so the parent can later retrieve that information with `wait()` or `waitpid()`. As long as the zombie is not reaped, it still consumes a slot in the kernel process table. So the practical rule is: *if a parent creates children, it should eventually wait for them*.

```c
pid_t pid = fork();

if (pid == 0) {
    _exit(0);
} else {
    sleep(10);   // parent does not wait yet
}
```

During those 10 seconds, the child may already be dead, but because the parent has not waited, the child can remain as a zombie. If the parent later does:

```c
wait(NULL);
```

then the zombie is reaped.

#### B.4.3 Blocking Behavior

By default, `wait()` is a **blocking** operation.

That means if the matching child has not yet finished, the parent stops running at that point until the child changes state in a relevant way. The man page says `wait()` suspends execution of the calling thread until one of its children terminates, and `waitpid()` similarly suspends execution until a selected child changes state.

```c
pid_t pid = fork();

if (pid == 0) {
    sleep(2);
    _exit(0);
} else {
    printf("parent waiting...\n");
    wait(NULL);
    printf("child finished\n");
}
```

The parent will print:

```text
parent waiting...
```

then it pauses for about 2 seconds, then:

```text
child finished
```

because `wait()` blocks.

#### B.4.4 The Non-blocking Flag: `WNOHANG`

If `WNOHANG` is used, `waitpid()` returns immediately if no matching child has exited yet. It returns `0` in that case.

```c
pid_t pid = fork();

if (pid == 0) {
    sleep(5);
    _exit(0);
} else {
    pid_t result = waitpid(pid, NULL, WNOHANG);

    if (result == 0) {
        printf("child has not exited yet\n");
    } else if (result == pid) {
        printf("child already finished\n");
    }
}
```

This parent does **not** block. It just checks whether the child is finished yet. So:

- `waitpid(pid, NULL, 0)` → blocking
- `waitpid(pid, NULL, WNOHANG)` → non-blocking check

This is useful if the parent wants to keep doing other work while occasionally checking whether a child has finished.

#### B.4.5 `wait()` with `sleep()` and `wait()` with `fork()`

This connects directly to the earlier fork example.

In the scaffold:

```c
if (pid == 0) {
    puts("child");
    n++;
    // sleep(1);
} else {
    puts("parent");
    n *= 2;
    // wait(NULL);
}
```

`wait(NULL)` is inside the parent branch **after**:

```c
puts("parent");
n *= 2;
```

So `wait(NULL)` does **not** force `"child"` to appear before `"parent"`.

It only forces the parent’s **final** `printf(...)` to happen after the child has finished.

So with `wait(NULL)`, we can still get:

```text
about to fork
parent
child
fork returned 0, n is 5
fork returned 12345, n is 8
```

or:

```text
about to fork
child
parent
fork returned 0, n is 5
fork returned 12345, n is 8
```

What `wait()` really orders is the code **after the wait call**.

#### B.4.6 Getting the child's Exit Status

If we pass a real `int *` instead of `NULL`, then `wait()` / `waitpid()` store status information there.

Example:

```c
int status;
pid_t done = wait(&status);
```

Then we can inspect `status` using macros such as:

- `WIFEXITED(status) -> child exited normally`
- `WEXITSTATUS(status) -> child's exit code`
- `WIFSIGNALED(status) -> child was terminated by a signal`
- `WTERMSIG(status) -> which signal killed it`

```c
pid_t pid = fork();

if (pid == 0) {
    _exit(7);
} else {
    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        printf("child exited with %d\n", WEXITSTATUS(status));
    }
}
```

This will print:

```text
child exited with 7
```

That is often much more useful than just `wait(NULL)`.

#### B.4.7 Other `waitpid()` Options

Besides `WNOHANG`, two other common options are:

- `WUNTRACED` → also return if a child has stopped
- `WCONTINUED` → also return if a stopped child has resumed with `SIGCONT`

---

### B.5 `exec*()`

The `exec` family lets a process replace its **current program image** with a new one. The most important correction to keep in mind is that `exec` does **not** create a new process. It reuses the existing process: the PID stays the same, but the running program, stack, heap, and data segments are replaced by the new program image. On success, `exec` does not return. So the core idea is: `fork()` creates a new process and `exec()` replaces the current process's program image.

That is why shells usually do both: they `fork()` first to create a child, then the child `exec()`s the command the user asked for. That keeps the parent shell alive while the child turns into the new program.

After a successful `execve()`, the current process starts running a different program. The new program gets a newly initialized stack, heap, and data segments, and the old text/data/bss/stack of the caller are overwritten by the newly loaded program. The `execve(2)` man page also points out that calling this "executing a new process" is misleading, because there is no new process; the existing process just starts executing a new program.

#### B.5.1 The Scaffold Example

```c
#include <stdio.h>
#include <unistd.h>

int main(void) {
    puts("about to launch sort");

    if (execl("/usr/bin/sort", "sort", "forkdemo.c", NULL) == -1) {
        perror("exec failed");
        return 1;
    }

    puts("finished sort");
    return 0;
}
```

**Step 1: before `exec`**

The program first prints:

```text
about to launch sort
```

This definitely happens, because it is before the `execl(...)` call.

**Step 2: the `execl(...)` call**

```c
execl("/usr/bin/sort", "sort", "forkdemo.c", NULL)
```

This asks the current process to run the executable at `/usr/bin/sort`. The argument list passed to the new program is:

- `argv[0] = "sort"`
- `argv[1] = "forkdemo.c"`
- then a terminating null pointer

The `exec(3)` man page says the `'l'` forms take arguments as a list (`arg0, arg1, ...`) and that the list must be terminated by a null pointer. By convention, the first argument should be the filename associated with the file being executed.

So the new program behaves as if the shell had run:

```bash
sort forkdemo.c
```

**Step 3: why `"finished sort"` is not printed**

This line:

```c
puts("finished sort");
```

runs **only if `execl` fails**.

That is because the `exec` functions return only when an error occurs. On success, the current process image is replaced, so control never comes back to the old program.

If `exec` fails, then it returns `-1`, `perror("exec failed")` runs, and the program exits with failure.

#### B.5.2 How Shells Launch Commands

If we write only:

```c
execl("/usr/bin/sort", "sort", "forkdemo.c", NULL);
```

then my current process stops being my original program and becomes `sort`.

But often we want:

- parent keeps running
- child becomes the new program

That is the classic `fork + exec` pattern:

```c
pid_t pid = fork();

if (pid == 0) {
    execl("/usr/bin/sort", "sort", "forkdemo.c", NULL);
    perror("exec failed");
    _exit(1);
} else if (pid > 0) {
    wait(NULL);
}
```

Now:

- parent survives
- child becomes `sort`
- parent can wait for child to finish

This is how shells launch commands.

#### B.5.3 The `exec` Family

There is not just one `exec` function. There is a whole family:

- `execl`
- `execv`
- `execlp`
- `execvp`
- `execle`
- `execvpe`
- and the underlying system-call-level form `execve`

The `exec(3)` man page groups them by the letters after `exec`.

##### `l` Means "List"

The arguments are written directly in the function call:

```c
execl("/bin/echo", "echo", "hello", "world", (char *)NULL);
```

This is convenient when the number of arguments is fixed and small.

> [!IMPORTANT]
> Refer to [`execl.c`](./Codes/execl.c) for code demonstration.

##### `v` Means "Vector"

The arguments are passed as an `argv` array:

```c
char *args[] = {"echo", "hello", "world", NULL};
execv("/bin/echo", args);
```

This is useful when the argument list is already stored in an array or built dynamically. The array must end with a null pointer.

> [!IMPORTANT]
> Refer to [`execv.c`](./Codes/execv.c) for code demonstration.

##### `p` Means "Search PATH"

The `p` forms search the directories in the `PATH` environment variable if the filename does not contain a slash.

So:

```c
execlp("sort", "sort", "forkdemo.c", (char *)NULL);
```

can find `sort` through `PATH`, instead of requiring the full path `/usr/bin/sort`. If the filename contains a slash, `PATH` is ignored.

> [!IMPORTANT]
> Refer to [`execvp.c`](./Codes/execvp.c) for code demonstration.

##### `e` Means "Explicit Environment"

The `e` forms let us pass an explicit environment array `envp` to the new program, instead of inheriting the caller's environment.

```c
char *args[] = {"printenv", NULL};
char *env[] = {"MYVAR=hello", NULL};
execve("/usr/bin/printenv", args, env);
```

The `exec(3)` and `execve(2)` man pages say the environment array must also be terminated by a null pointer. If we do not use an `e` form, the new program inherits the calling process's environment.

##### `argv` And `argv[0]`

`exec` does not build the argument vector automatically for us.

- `argv[0]` should be the program name associated with the file being executed
- the argument list must end with `NULL`

```c
execl("/usr/bin/sort", "sort", "forkdemo.c", (char *)NULL);
```
the new program sees:

```text
argv[0] = "sort"
argv[1] = "forkdemo.c"
argv[2] = NULL
```

The `execve(2)` page says the new program accesses these as its command-line arguments, and `argv[argc]` will be a null pointer.

##### `execve()` is the Underlying Form

The higher-level `exec` functions are wrappers around `execve()`. The man page states that the `exec()` family is layered on top of `execve(2)`. So in practice:

- `execl`, `execv`, `execvp`, and friends are convenience interfaces
- `execve` is the core system-call-level interface

Its prototype is:

```c
int execve(const char *path, char *const argv[], char *const envp[]);
```

#### B.5.4 What Stays the Same Across `exec` and What Not

Across `exec`, those stay same:

- the process itself
- its PID

while A successful `execve()` resets or discards several process attributes.

- caught signal dispositions are reset to default
- alternate signal stacks are not preserved
- memory mappings are not preserved
- POSIX shared memory mappings are unmapped
- POSIX timers are not preserved
- exit handlers are not preserved

This is especially important for later signal notes: *if a process installs a custom signal handler and then `exec()`s a new program, caught signal handlers do not carry over; they are reset to default behavior.* So `exec` is a much stronger reset than `fork`.

---

### B.6 Exercise: Bash Simulator

> [!IMPORTANT]
> Refer to [`bash_simulator.c`](./Codes/bash_simulator.c) for the code used in this section.

The goal is to build a very small shell loop:

1. print a prompt
2. read one command line
3. `fork()`
4. in the child, `exec...` the command
5. in the parent, `wait...` for the child
6. then show the prompt again

That is exactly the basic pattern a real shell uses. `fork()` creates the child, the parent gets the child PID while the child gets `0`, and parent and child then run in separate memory spaces. The `exec` family replaces the current process image with a new program image, and the `exec` functions return only on error. `waitpid(pid, ..., 0)` blocks until that specific child changes state, which is why the terminal "pauses" until the command finishes.

> [!NOTE]
> Even though it says "only execute processes that have no output", the child actually inherits copies of the parent's open file descriptors after `fork()`, so normal output still goes to the same terminal unless you do something special with redirection or pipes. We are just not implementing custom shell I/O features yet. If you want to keep testing simple, commands like `sleep 1`, `true`, `false`, or `mkdir demo` are good examples.

#### B.6.1 Read One Line and Split It into Arguments

We read a command line from standard input with `fgets`. If the line is empty, we ignore it. Then, we split it on spaces and tabs. That means this mini-shell does **not** handle quotes or escapes yet, but it is enough for simple commands like:

```bash
ls
sleep 1
mkdir test
```

We store the tokens in an `argv` array and terminate it with `NULL`, because the `v` forms of `exec` expect a null-terminated argument vector.

#### B.6.2 Handle `exit` inside the Shell Itself

`exit` should be a built-in command. If the user types `exit`, the shell loop stops instead of trying to run an external program called `exit`.

#### B.6.3 Next Step: `fork()`

After parsing, call `fork()`.

- if `fork()` returns `< 0`, it failed
- if it returns `0`, we are in the child
- otherwise, we are in the parent, and the return value is the child PID

#### B.6.4 Then Child Uses `execvp()`

In the child, call:

```c
execvp(argv[0], argv);
```

This is the nicest choice here because:

- `v` means arguments are passed as an `argv[]` array
- `p` means it searches `PATH` like a shell when the filename has no slash

That makes it much more convenient than hardcoding full paths like `/usr/bin/sort`. If `execvp()` succeeds, the child process is replaced by the new program and never returns to our shell code. If it returns, that means there was an error.

#### B.6.5 Meanwhile Parent Waits

In the parent, use:

```c
waitpid(pid, NULL, 0);
```

This blocks until that specific child finishes. That is what makes the mini-shell wait before showing the next prompt. `waitpid()` with `WNOHANG` would be the non-blocking version, but for this exercise we want the normal blocking behavior.
