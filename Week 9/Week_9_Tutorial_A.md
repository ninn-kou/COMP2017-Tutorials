## COMP2017 2026 S1 Week 9 Tutorial A

<table><tbody>
  <tr><td><b>Tutor</b></td><td>Hao Ren</td></tr>
  <tr><td><b>Email</b></td><td><a href="hao.ren@sydney.edu.au">hao.ren@sydney.edu.au</a></td></tr>
</tbody></table>

- [COMP2017 2026 S1 Week 9 Tutorial A](#comp2017-2026-s1-week-9-tutorial-a)
  - [A.1 Exercise: "DO YOUR HOMEWORK" Parent/Child Signal Dialogue](#a1-exercise-do-your-homework-parentchild-signal-dialogue)
    - [A.1.1 Think Before You Go: Why `pause()` Can be Buggy](#a11-think-before-you-go-why-pause-can-be-buggy)
    - [A.1.2 Design Pattern](#a12-design-pattern)
    - [A.1.3 Step 1: Create the Signal Set](#a13-step-1-create-the-signal-set)
    - [A.1.4 Step 2: Fork the Child Process](#a14-step-2-fork-the-child-process)
    - [A.1.5 Step 3: Use Signals to Pass Turns](#a15-step-3-use-signals-to-pass-turns)
  - [A.2 Safe Fork + Exec](#a2-safe-fork--exec)
    - [A.2.1 `popen()` and `pclose()`](#a21-popen-and-pclose)
    - [A.2.2 `posix_spawn()`](#a22-posix_spawn)
  - [A.3 Exercise: Logger](#a3-exercise-logger)
    - [A.3.1 Try This Exercise by Yourself! Hints: Design Choice](#a31-try-this-exercise-by-yourself-hints-design-choice)
    - [A.3.2 Step 1: Parse `--cmd` and `--out`](#a32-step-1-parse---cmd-and---out)
    - [A.3.3 Step 2: Start the Command with `popen()`](#a33-step-2-start-the-command-with-popen)
    - [A.3.4 Step 3: Copy the Output into a File](#a34-step-3-copy-the-output-into-a-file)
    - [A.3.5 Extension: Also Capturing `stderr`](#a35-extension-also-capturing-stderr)
  - [A.4 Exercise: Parallel Search](#a4-exercise-parallel-search)
    - [A.4.1 Step 1: Fork One Child per File](#a41-step-1-fork-one-child-per-file)
    - [A.4.2 Step 2: Use `exec` in the Child](#a42-step-2-use-exec-in-the-child)
    - [A.4.3 Step 3: Parent Waits for All Children](#a43-step-3-parent-waits-for-all-children)
  - [A.5 `sigaction()`](#a5-sigaction)
    - [A.5.1 Core Concept](#a51-core-concept)
    - [A.5.2 The `struct sigaction`](#a52-the-struct-sigaction)
    - [A.5.3 Simple Handler: `sa_handler`](#a53-simple-handler-sa_handler)
    - [A.5.4 Extended Handler: `sa_sigaction` and `SA_SIGINFO`](#a54-extended-handler-sa_sigaction-and-sa_siginfo)
    - [A.5.3 `sa_mask`: Signals Blocked While the Handler Runs](#a53-sa_mask-signals-blocked-while-the-handler-runs)
    - [A.5.4 Signal Mask versus `sa_mask`](#a54-signal-mask-versus-sa_mask)
    - [A.5.5 Useful Flags](#a55-useful-flags)
      - [`SA_SIGINFO`](#sa_siginfo)
      - [`SA_RESTART`](#sa_restart)
      - [`SA_NOCLDSTOP`](#sa_nocldstop)
      - [`SA_NODEFER`](#sa_nodefer)
    - [A.5.6 Important Safety Rule: Keep Handlers Small](#a56-important-safety-rule-keep-handlers-small)
    - [A.5.7 `SIGKILL` and `SIGSTOP`](#a57-sigkill-and-sigstop)
  - [A.6 Exercise: `SIGCHLD` - Tracking Children](#a6-exercise-sigchld---tracking-children)
    - [A.6.1 Hints: What Is Our Core Idea?](#a61-hints-what-is-our-core-idea)
    - [A.6.2 Summary](#a62-summary)
  - [A.7 Exercise: `SIGSEGV` I - Coredump](#a7-exercise-sigsegv-i---coredump)
    - [A.7.1 Core Concept](#a71-core-concept)
    - [A.7.2 Do Not Return from the Handler](#a72-do-not-return-from-the-handler)
    - [A.7.3 Summary](#a73-summary)
  - [A.8 Exercise: `SIGSEGV` II - Page Fault](#a8-exercise-sigsegv-ii---page-fault)
    - [A.8.1 Core Idea to My Solution](#a81-core-idea-to-my-solution)

---

### A.1 Exercise: "DO YOUR HOMEWORK" Parent/Child Signal Dialogue

The goal is to create two processes:

```text
parent process  -> prints the parent lines
child process   -> prints the child lines
```

The two processes must take turns. The parent should not print all of its lines first, and the child should not speak before the parent has asked the first question.

The signal is used as a simple "your turn now" message.

In this solution, `SIGINT` is the turn-taking signal. The parent sends `SIGINT` to the child, and the child sends `SIGINT` back to the parent using `kill(pid, SIGINT)`. On Linux, `kill()` sends a signal to a process or process group; when the PID is positive, it sends the signal to that specific process.

> [!IMPORTANT]
> Refer to [`homework.c`](./Codes/homework.c) for the code used in this section.

#### A.1.1 Think Before You Go: Why `pause()` Can be Buggy

> [! CAUTION]
> The important subtlety is that this should not be implemented with plain `pause()` unless we are very careful. `pause()` sleeps until a signal is delivered and a handler runs, but it does not remember that a signal already arrived before the `pause()` call.

So instead, we use `sigwait()` in our solutions.

A tempting approach is:

```c
pause();
printf("my turn\n");
```

The problem is timing.

Suppose the other process sends the signal slightly before this process calls `pause()`:

```text
signal arrives
handler runs
process then calls pause()
process sleeps forever
```

The signal already happened, so `pause()` is now waiting for another signal. If no more signal is sent, the program blocks indefinitely.

This is a classic lost wake-up race.

At a conceptual level:

```text
pause() waits for a future signal.
pause() does not check whether the signal already happened.
```

A safer design is to block the signal first, then wait for it using `sigwait()`. `sigwait()` waits until a signal from a given signal set becomes pending, accepts it, removes it from the pending signal list, and returns the signal number. For this to work correctly, the signal being waited for should be blocked first.

#### A.1.2 Design Pattern

 This solution uses this pattern:

```c
sigemptyset(&turn_set);
sigaddset(&turn_set, SIGINT);
sigprocmask(SIG_BLOCK, &turn_set, NULL);
```

Then both processes use:

```c
sigwait(&turn_set, &received_signal);
```

This means:

```text
SIGINT is blocked normally.
If SIGINT arrives early, it becomes pending.
sigwait() collects the pending SIGINT safely.
```

So the signal is not lost if it arrives before the process reaches the waiting line.

> [!CAUTION]
> A mutex is not usually the right tool for this specific parent/child signal problem unless we are using shared memory or process-shared synchronization. Busy waiting can avoid blocking forever, but it wastes CPU. `sigwait()` or `sigsuspend()` is the cleaner signal-based solution.

#### A.1.3 Step 1: Create the Signal Set

We first create a signal set containing only `SIGINT`.

```c
sigset_t turn_set;

sigemptyset(&turn_set);
sigaddset(&turn_set, SIGINT);
```

`sigemptyset()` initializes a signal set to contain no signals, and `sigaddset()` adds one signal to the set. A `sigset_t` should be initialized before being used.

Then we block that signal:

```c
sigprocmask(SIG_BLOCK, &turn_set, NULL);
```

Blocking `SIGINT` does not throw it away. It prevents normal delivery, so if it arrives, it becomes pending until we accept it with `sigwait()`.

#### A.1.4 Step 2: Fork the Child Process

The parent gets its PID before `fork()`:

```c
pid_t parent_pid = getpid();
pid_t child_pid = fork();
```

`getpid()` returns the PID of the calling process. This lets the child know where to send the signal back.

After `fork()`:

```text
child_pid == 0      means this is the child process
child_pid > 0       means this is the parent process
child_pid == -1     means fork failed
```

The child inherits the blocked `SIGINT` mask, which is useful here because both parent and child need the same turn-taking mechanism.

#### A.1.5 Step 3: Use Signals to Pass Turns

The parent starts the conversation:

```text
P: Have you done your homework?
```

Then the parent sends `SIGINT` to the child.

The child waits for `SIGINT`, prints its reply, then sends `SIGINT` back to the parent.

The pattern is:

```text
print my line
signal the other process
wait for my next turn
```

At the end, the parent calls `waitpid()` to reap the child process. `waitpid()` waits for a child process to change state, and waiting for a terminated child lets the system release the child's remaining process resources; otherwise a terminated child can remain as a zombie.

---

### A.2 Safe Fork + Exec

Creating a new program from C often means combining two ideas:

```text
fork()  -> create a child process
exec()  -> replace the child process image with a new program
```

This is powerful, but it is easy to make mistakes. The most common mistakes are:

```text
not checking fork()
forgetting that both parent and child continue after fork()
forgetting that exec() can fail
forgetting to wait for children
accidentally involving the shell when it is not needed
```

#### A.2.1 `popen()` and `pclose()`

> [!NOTE]
> We will learn more about pipelines in [Tutorial B](Week_9_Tutorial_B.md) this week.

`popen()` creates a pipe, forks, and invokes the shell. The returned `FILE *` is connected to either the command's standard output or standard input. Because the pipe is unidirectional, the mode can be for reading or writing, but not both.

For example,

```c
// let my program read from the command's stdout
FILE *f = popen("ls -l", "r");
```

```c
// let my program write to the command's stdin
FILE *f = popen("sort", "w");
```

> [!WARNING]
> popen() is convenient, but it is not the safest choice for untrusted input.

#### A.2.2 `posix_spawn()`

`posix_spawn()` is another way to create a new process that runs a program.

It can be thought of as a more controlled helper for the common:

```text
fork() + exec()
```

pattern.

Unlike `popen()`, it does not require passing one big shell command string. Instead, it uses an explicit `argv` array. The Linux man page describes `posix_spawn()` and `posix_spawnp()` as functions used to create a new child process that executes a specified file.

> [!IMPORTANT]
> Refer to [`posix_spawn.c`](./Codes/posix_spawn.c) for demonstration.

---

### A.3 Exercise: Logger

> [!IMPORTANT]
> Refer to [`proc_log.c`](./Codes/proc_log.c) for the solution to this exercise.

The goal is to write a program called `proc_log` that runs another command and saves that command's output into a file. Usage example should looks like:

```bash
./proc_log --cmd="python3 say_hello_100.py" --out=test.out
```

It should mean the program will run `python3 say_hello_100.py` and capture its `stdout`. Then write the `stdout` into `test.out`.

`popen()` lets our C program start another command and treat that command's `stdout` or `stdin` as a `FILE *`. For this exercise, we want to read from the command, so use mode `r`:

```c
popen(command, "r");
```

That gives me a stream connected to the command's standard output.

#### A.3.1 Try This Exercise by Yourself! Hints: Design Choice

You should try to solve this exercise by yourself before you visit my solution. Think the program's steps first! Following is the logic of this program and you could use them as a hint.

<details>
  <summary><b>Hints</b>: Click to Expand</summary>
  <ol>
    <li>parse <code>--cmd</code> and <code>--out</code></li>
    <li>open the output file</li>
    <li>start the command with <code>popen(cmd, "r")</code></li>
    <li>copy bytes from the command output into the file</li>
    <li>close the output file</li>
    <li>close the command stream with <code>pclose()</code></li>
    <li>check the command exit status</li>
  </ol>
  <p>The program does not need to manually use <code>fork()</code>, <code>pipe()</code>, <code>dup2()</code>, or <code>exec()</code> because <code>popen()</code> handles that pattern internally.</p>
  <p>One important detail: <code>popen()</code> runs the command through the shell. That is convenient because this works.</p>
  <p>But it also means untrusted user input is dangerous, because shell syntax could be interpreted.</p>
</details>

#### A.3.2 Step 1: Parse `--cmd` and `--out`

The program expects arguments in this form:

```bash
--cmd="<command>"
--out=<output-file>
```

For example:

```bash
./proc_log --cmd="ls -l" --out=listing.txt
```

The shell removes the quotes before the program receives the argument, so inside `argv`, the command looks like this:

```text
--cmd=ls -l
```

That is still one argument because the quotes grouped it in the shell.

A common mistake is writing:

```bash
./proc_log --cmd=python3 say_hello_100.py --out=test.out
```

This is not the same thing. The shell may split it into separate arguments:

```text
--cmd=python3
say_hello_100.py
--out=test.out
```

So commands containing spaces should be quoted.

#### A.3.3 Step 2: Start the Command with `popen()`

For this logger, we want to read the command's output:

```c
FILE *cmd_stream = popen(cmd, "r");
```

The mode `"w"` or `"r"` means:

```text
r: my program reads from the command's stdout
w: my program writes to the command's stdin
```

#### A.3.4 Step 3: Copy the Output into a File

The logger should copy the output from the command into the output file.

Using `fread()` and `fwrite()` is a good choice because it copies bytes in chunks:

```c
while ((nread = fread(buffer, 1, sizeof buffer, cmd_stream)) > 0) {
    fwrite(buffer, 1, nread, out_file);
}
```

This is better than reading character-by-character.

It also works for normal text output and most simple binary output. In this exercise, the command output is probably text, but copying bytes is still a clean habit.

#### A.3.5 Extension: Also Capturing `stderr`

By default, this program captures only `stdout`. So if the command prints errors to `stderr`, those errors still appear in the terminal.

For example:

```bash
./proc_log --cmd="python3 broken.py" --out=test.out
```

If `broken.py` writes to `stderr`, that output may not go into `test.out`.

Because `popen()` uses the shell, we can redirect `stderr` into `stdout` in the command string:

```bash
./proc_log --cmd="python3 broken.py 2>&1" --out=test.out
```

Now both `stdout` and `stderr` are captured into the output file.

The important distinction is:

- normal `popen(cmd, "r")` captures `stdout`
- `cmd 2>&1` makes `stderr` also go to `stdout`

---

### A.4 Exercise: Parallel Search

> [!IMPORTANT]
> Refer to [`parasearch.c`](./Codes/parasearch.c) for the solution to this exercise.

```bash
./parasearch word file1 file2 file3 ...
```

Each child searches one file. The parent waits for all children, prints whether each file contains the word, and finally prints `process complete`.

To keep the solution short, we can let `grep` do the actual searching. Each child runs:

```bash
grep -F -q -- word filename
```

- `-F` means search for the word as plain text, not a regular expression.
- `-q` means quiet mode: `grep` does not print matching lines, but its exit status tells us the result.

#### A.4.1 Step 1: Fork One Child per File

For each file:

```c
pid_t pid = fork();
```

The child runs `grep`.

The parent stores the child PID so it can later match the finished child back to the correct filename.

#### A.4.2 Step 2: Use `exec` in the Child

The child replaces itself with `grep`:

```c
execlp("grep", "grep", "-F", "-q", "--", word, filename, NULL);
```

If `execlp()` succeeds, the child is now `grep`.

If `execlp()` fails, the child must exit immediately:

```c
perror("execlp");
_exit(127);
```

#### A.4.3 Step 3: Parent Waits for All Children

`grep` uses these exit codes:

```text
0  found
1  not found
2+ error
```

So the parent waits for each child, checks its exit status, and prints the result.

> [!CAUTION]
> A common mistake is waiting immediately after each `fork()`:
>
> ```c
> fork();
> wait(NULL);
> ```
>
> That makes the searches mostly sequential, not parallel. The better pattern is:
>
> ```text
> fork all children first
> then wait for them
> ```
>
> Another common mistake is forgetting that `exec` can fail. Code after `execlp()` is the error path and should call `_exit()`.
>
>Also, the output order is not guaranteed. Parallel programs often finish in a different order from the input files.

---

### A.5 `sigaction()`

`sigaction()` is the preferred POSIX way to install a signal handler.

Earlier examples often use:

```c
signal(SIGINT, handler);
```

That is simple, but it hides details and can behave differently across systems. The only fully portable uses of `signal()` are setting a signal to its default action with `SIG_DFL` or ignoring it with `SIG_IGN`; using `signal()` to install a custom handler has historically varied across systems. `sigaction()` gives more explicit control over how the handler is installed and invoked.

#### A.5.1 Core Concept

A signal has an action, sometimes called its disposition. That action says what should happen when the process receives that signal.

For example, a signal might:

```text
terminate the process
be ignored
run a custom handler function
```

`sigaction()` changes the action for a specific signal:

```c
int sigaction(int signum,
              const struct sigaction *act,
              struct sigaction *oldact);
```

The arguments are:

```text
signum  the signal being configured, such as SIGINT or SIGUSR1
act     the new action to install
oldact  where the old action can be saved
```

If `act` is non-`NULL`, `sigaction()` installs the new action. If `oldact` is non-`NULL`, the previous action is written there. For most tutorial programs, passing `NULL` for `oldact` is fine because we do not need to restore the old handler. `sigaction()` returns `0` on success and `-1` on error.

#### A.5.2 The `struct sigaction`

The structure looks roughly like this:

```c
struct sigaction {
    void     (*sa_handler)(int);
    void     (*sa_sigaction)(int, siginfo_t *, void *);
    sigset_t   sa_mask;
    int        sa_flags;
};
```

There may be some system-specific fields, but these are the ones that matter for this course.

> [!CAUTION]
> Do not assign both `sa_handler` and `sa_sigaction`. On some systems, they may share storage internally, so choose one style of handler.

#### A.5.3 Simple Handler: `sa_handler`

The simple handler form takes one argument:

```c
void handler(int signum);
```

Example:

```c
#include <signal.h>

static volatile sig_atomic_t got_sigint = 0;

static void handle_sigint(int signum) {
    (void)signum;
    got_sigint = 1;
}
```

The handler receives the signal number. In this example, the handler only sets a flag. That is a good habit because signal handlers should stay small.

To install it:

```c
struct sigaction sa;

sa.sa_handler = handle_sigint;
sigemptyset(&sa.sa_mask);
sa.sa_flags = SA_RESTART;

if (sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigaction");
}
```

This means:

```text
when SIGINT arrives, call handle_sigint
while the handler is running, block only the default extra signals from sa_mask
use SA_RESTART behaviour where possible
```

#### A.5.4 Extended Handler: `sa_sigaction` and `SA_SIGINFO`

The extended handler form takes three arguments:

```c
void handler(int signum, siginfo_t *info, void *context);
```

To use this form, set:

```c
sa.sa_flags = SA_SIGINFO;
sa.sa_sigaction = handler;
```

When `SA_SIGINFO` is set, `sigaction()` uses `sa_sigaction` instead of `sa_handler`. The extra `siginfo_t *` can contain information such as the sending process ID, the sending user ID, an exit status for `SIGCHLD`, or the faulting address for signals such as `SIGSEGV`; which fields are meaningful depends on the signal and how it was generated.

Example:

```c
#include <signal.h>
#include <unistd.h>

static void handle_sigusr1(int signum, siginfo_t *info, void *context) {
    (void)signum;
    (void)context;

    pid_t sender = info->si_pid;

    /*
     * In a real handler, avoid printf().
     * This example only shows how to access the sender PID.
     */
    (void)sender;
}

int main(void) {
    struct sigaction sa;

    sa.sa_sigaction = handle_sigusr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;

    sigaction(SIGUSR1, &sa, NULL);

    for (;;) {
        pause();
    }
}
```

This form is useful when the signal itself is not enough information and we want to know more about where it came from.

> [!NOTE]
> **`sa_handler` versus `sa_sigaction`**
>
> Use `sa_handler` for a simple handler:
>
> ```c
> void handler(int signum);
> ```
>
> Use `sa_sigaction` only when `SA_SIGINFO` is set:
>
> ```c
> void handler(int signum, siginfo_t *info, void *context);
> ```
>
> Do not set both.

#### A.5.3 `sa_mask`: Signals Blocked While the Handler Runs

`sa_mask` is a set of signals that should be temporarily blocked while the handler is executing. Also, by default, the signal that triggered the handler is blocked while that handler is running. This prevents the same signal from interrupting its own handler again, unless `SA_NODEFER` is used.

Example:

```c
struct sigaction sa;

sa.sa_handler = handle_sigint;
sigemptyset(&sa.sa_mask);
sigaddset(&sa.sa_mask, SIGTERM);
sa.sa_flags = 0;

sigaction(SIGINT, &sa, NULL);
```

This means:

```text
when handling SIGINT, temporarily also block SIGTERM
```

So if `SIGTERM` arrives while the `SIGINT` handler is running, it will not interrupt that handler immediately.

> [!NOTE]
> **`sa_mask` versus blocking a signal normally**
>
> ```c
> sigprocmask(SIG_BLOCK, &set, NULL);
> ```
>
> blocks signals now.
>
> This:
>
> ```c
> sa.sa_mask = set;
> ```
>
> blocks those signals only while the handler is running.

#### A.5.4 Signal Mask versus `sa_mask`

This is a common confusion point.

The signal mask is the set of signals currently blocked for a thread. `sigprocmask()` can inspect or change that mask; in Linux/POSIX terminology, each thread has its own signal mask, although in a single-threaded program it is fine to think of it as "the program's current blocked-signal set."

`sa_mask` is different. It does not permanently block signals. It says *while this handler is running, temporarily add these signals to the current signal mask*.

So:

```c
// changes the current blocked-signal set directly
sigprocmask(...)
// configures extra signals to block only during that handler
sa.sa_mask
```

#### A.5.5 Useful Flags

##### `SA_SIGINFO`

Use this when the handler needs extra information:

```c
sa.sa_flags = SA_SIGINFO;
sa.sa_sigaction = handler;
```

The handler uses this signature:

```c
void handler(int signum, siginfo_t *info, void *context);
```

This is useful for signals such as `SIGCHLD`, where information about the child process may matter, or for debugging signals such as `SIGSEGV`, where the faulting address may matter.

##### `SA_RESTART`

Some blocking system calls can be interrupted by a signal. With `SA_RESTART`, certain interrupted system calls may be automatically restarted instead of returning early with `EINTR`.

Example idea:

```c
sa.sa_flags = SA_RESTART;
```

This is often convenient when a signal handler only sets a flag and we do not want normal blocking I/O to fail just because a signal arrived.

The subtle point is that `SA_RESTART` does not mean every possible blocking operation is always restarted. It means the system uses restart behaviour for certain system calls.

> [!NOTE]
> **`SA_RESTART` versus "signals cannot interrupt anything"**
>
> `SA_RESTART` only affects certain interrupted system calls. It does not make signal handling invisible, and it does not mean all functions are safe to use in a handler.

##### `SA_NOCLDSTOP`

This flag is only meaningful for `SIGCHLD`.

Normally, a parent may receive `SIGCHLD` when a child terminates, and depending on settings, when a child stops or resumes. With `SA_NOCLDSTOP`, the parent does not receive `SIGCHLD` merely because a child stopped or resumed; the flag is intended for the case where we care about child termination rather than job-control stops.

Example:

```c
sa.sa_handler = handle_sigchld;
sigemptyset(&sa.sa_mask);
sa.sa_flags = SA_NOCLDSTOP;

sigaction(SIGCHLD, &sa, NULL);
```

This is useful when a program only wants to react when child processes actually finish.

##### `SA_NODEFER`

By default, when a signal handler is running, the signal that triggered it is blocked until the handler returns. `SA_NODEFER` disables that default blocking for the triggering signal, unless the signal is explicitly listed in `sa_mask`. This means the same signal may interrupt its own handler.

Example idea:

```c
sa.sa_flags = SA_NODEFER;
```

This is rarely needed in beginner-level code because it can create re-entrant handler calls:

```text
SIGINT arrives
handler starts
another SIGINT arrives
same handler starts again before the first one finished
```

That can make bugs much harder to reason about.

#### A.5.6 Important Safety Rule: Keep Handlers Small

Signal handlers run asynchronously. They can interrupt the program almost anywhere, including in the middle of library code.

That is why handler code should usually do something minimal, such as setting a `volatile sig_atomic_t` flag, writing a short message with an async-signal-safe function, or recording that cleanup is needed later.

Avoid this inside handlers:

```c
printf("got signal\n");
malloc(100);
free(ptr);
exit(1);
```

Many functions are not async-signal-safe. In particular, stdio functions such as `printf()` are unsafe in signal handlers because they use internal buffered state that may already be partially updated when the signal interrupts the program.

Better pattern:

```c
static volatile sig_atomic_t got_signal = 0;

static void handler(int signum) {
    (void)signum;
    got_signal = 1;
}
```

Then the main program checks the flag and performs normal work outside the handler.

#### A.5.7 `SIGKILL` and `SIGSTOP`

Some signals cannot be caught, ignored, or blocked. In particular, attempts to change the action for `SIGKILL` or `SIGSTOP` are invalid, and attempts to block them are ignored.

So this is not useful:

```c
sigaction(SIGKILL, &sa, NULL);
```

The operating system does not allow a process to handle `SIGKILL`.

---

### A.6 Exercise: `SIGCHLD` - Tracking Children

> [!WARNING]
> The signal is called `SIGCHLD`, not `SIGCHILD`.

> [!IMPORTANT]
> Refer to [`sigchld.c`](./Codes/sigchld.c) for the solution to this exercise.

The goal is to fork three children and let the parent track them using `SIGCHLD`.

Each child does this:

```text
sleep for a different number of seconds
then exit with a different status
```

The parent does not wait for all children in a normal blocking `wait()` loop. Instead, it installs a `SIGCHLD` handler. When a child finishes, the kernel sends `SIGCHLD` to the parent, and the handler reaps that child with `waitpid()`.

#### A.6.1 Hints: What Is Our Core Idea?

We use `sigaction()` with `SA_SIGINFO` so the handler receives a `siginfo_t *`, which can tell us:
1. which child changed state
2. whether it exited normally or was killed by a signal
3. the exit status or signal number

We also use `SA_NOCLDSTOP` so the parent is not notified just because a child was stopped.

Instead of using plain `pause()` forever, we could uses a `children_left` counter and `sigsuspend()`. This lets the parent stop once all children have been reaped.

#### A.6.2 Summary

`SIGCHLD` lets a parent notice when children finish. With `sigaction()` and `SA_SIGINFO`, the handler can inspect `siginfo_t` to find the child PID and whether the child exited normally or died from a signal.

The handler reaps the child using `waitpid()`, which prevents zombies. `SA_NOCLDSTOP` avoids unnecessary notifications when children merely stop.

The parent forks all three children, then waits for the handler to process them. Once all three have been reaped, the parent exits cleanly.

---

### A.7 Exercise: `SIGSEGV` I - Coredump

The goal is to install a custom `SIGSEGV` handler so that when the program segfaults, it prints useful debugging information immediately. Normally, a segmentation fault just terminates the program and may create a core dump. With `sigaction()` and `SA_SIGINFO`, the handler can inspect a `siginfo_t *` and print more detail.

> [!IMPORTANT]
> Refer to [`coredump.c`](./Codes/coredump.c) for the solution to this exercise.

#### A.7.1 Core Concept

`SIGSEGV` happens when a process makes an invalid memory access.

Common causes include:

```text
dereferencing NULL
accessing an unmapped address
writing to read-only memory
accessing memory after it has been unmapped
```

With `SA_SIGINFO`, the handler receives this extra information:

```c
info->si_addr    // the address that caused the fault
info->si_code    // explains the kind of fault
```

The two important `SIGSEGV` codes here are:

```text
SEGV_MAPERR   address is not mapped
SEGV_ACCERR   address is mapped, but permissions were wrong
```

For example:

```c
int *p = NULL;
*p = 42;
```

usually gives `SEGV_MAPERR` because address `0x0` is not mapped.

And writing to a read-only `mmap()` page usually gives `SEGV_ACCERR` because the page exists, but writing to it is not allowed.

#### A.7.2 Do Not Return from the Handler

After a `SIGSEGV`, the program is sitting at the faulting instruction.

If the handler returns, the program normally retries the same bad instruction:

```text
fault
handler runs
handler returns
same bad memory access happens again
fault again
```

So the handler should terminate the process.

Use `_exit(1)` instead of `exit(1)`. Beacuse `_exit()` terminates immediately, and `exit()` may run cleanup handlers and flush stdio buffers, which is not ideal from inside a signal handler.

#### A.7.3 Summary

`SIGSEGV` is sent when a process performs an invalid memory access. Installing a handler with `sigaction()` and `SA_SIGINFO` lets the program inspect the fault using `siginfo_t`.

The key fields are:

```text
info->si_addr   address that caused the fault
info->si_code   reason for the fault
```

`SEGV_MAPERR` means the address was not mapped.
`SEGV_ACCERR` means the address existed, but the requested access was not permitted.

After printing the diagnostic message, the handler should call `_exit()` instead of returning.

---

### A.8 Exercise: `SIGSEGV` II - Page Fault

The goal is to deliberately create a memory region that exists in the process address space, but cannot be accessed. We do that with:

```c
mmap(NULL, page_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
```

This creates a page with no permissions: no read, no write, and no execute. So when we touch it, the hardware raises a page fault. The kernel sees that the page exists, but the access is not allowed, so it sends the process `SIGSEGV` with `SEGV_ACCERR`. The handler then changes the page permission using `mprotect()`. After the handler returns, the faulting instruction is retried and succeeds.

> [!IMPORTANT]
> Refer to [`page_fault.c`](./Codes/page_fault.c) for the solution to this exercise.

> [!NOTE]
> In the previous SIGSEGV exercise, the handler called `_exit(1)` because the fault could not be fixed. But in this exercise, the handler does not exit. Instead, it fixes the memory permission:
>
> ```c
> mprotect(page, page_size, PROT_READ | PROT_WRITE);
> ```
>
> Then it returns. This only works because the handler knows exactly what caused the fault and how to make the access valid.

#### A.8.1 Core Idea to My Solution

`mmap()` adds a new region to the process address space. Here, we ask for one page:

```c
page = mmap(NULL,
            page_size,
            PROT_NONE,
            MAP_PRIVATE | MAP_ANONYMOUS,
            -1,
            0);
```

The page exists, but it has no access permission. So `page[0] = 42` faults.

The kernel delivers `SIGSEGV`, and the handler receives extra information through `siginfo_t *info`. The two key fields are:

```c
info->si_addr    // gives the address that caused the fault
info->si_code    // tells us the reason
```

Furthermore, for this exercise:

```text
SEGV_MAPERR means the address was not mapped
SEGV_ACCERR means the address was mapped, but access was not allowed
```

Because the page was created with `PROT_NONE`, the expected result is `SEGV_ACCERR`.

The handler then fixes the permission:

```c
mprotect(page, page_size, PROT_READ | PROT_WRITE);
```

After the handler returns, the CPU retries the original instruction `page[0] = 42` And this time the page is writable, so the program continues.
